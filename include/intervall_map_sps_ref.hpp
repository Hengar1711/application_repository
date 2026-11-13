#pragma once
/*!
    @file intervall_map_sps.hpp
    @author your name (you@domain.com)
    @brief 
    @version 0.1
    @date 2026-11-13

    @copyright Copyright (c) 2026

*/


#include <map>
#include <chrono>
#include <queue>

#include <iostream>

namespace SPS
{
    /*!
        @brief Deterministic, state-machine-driven interval map wrapper optimized for real-time PLC and SPS execution environments.

        @details **Betrachtung:** 
                    Diese Klasse kapselt die klassische `interval_map`-Logik in ein zustandsbasiertes 
                    Framework (State Machine). Ziel ist es, rechenintensive Operationen (Suchen, Einfügen, 
                    Löschen) in deterministische Einzelschritte zu zerlegen, um Verzögerungsspitzen und harte 
                    Echtzeit-Latenzen im SPS-Zyklus zu verhindern.

        @details **Regeln:** 
                    - Die Ausführung erfolgt gepuffert über eine interne `assignqueue`.
                    - Zuweisungsaufträge müssen zyklisch über den `run()`-Hook abgearbeitet werden.
                    - Ungültige Intervalle (`!(keyBegin < keyEnd)`) werden sofort abgewiesen.

        @details **Erkenntnisse:** 
                    Durch die zustandsbasierte Aufteilung wird eine extrem niedrige und vorhersehbare 
                    Zykluszeit (~100 ns avg) erreicht. Manuelle Optimierungen der STL-Hint-Logik erwiesen 
                    sich als kontraproduktiv; die nativen Container-Innereien bilden in Kombination mit 
                    dieser State Machine den optimalen Leistungs-Sweet-Spot.

        @tparam K Type of the key elements.
        @tparam V Type of the stored values.
    */
    template <typename K, typename V>
    class interval_map
    {
        friend void IntervalMapTest();    

        bool Debugging = false;

        typedef std::pair<K,K> INTERVALLKEYS;
        typedef std::pair<INTERVALLKEYS,V> INTERVALL;

        std::queue<INTERVALL> assignqueue;

    public:
        std::map<K,V> &m_map;
        V &m_valBegin;

        typename std::map<K,V>::iterator Searchprev;
        typename std::map<K,V>::iterator Searchsection;
        typename std::map<K,V>::reverse_iterator Searchsectionreverse;
        typename std::map<K,V>::iterator Searchnext;

        K *FokusKey;
        V *val;
        

        bool END = false;
        bool EndHandled = false;
        bool direkt_both = false;
        bool erase_prev = false;
        bool Begin_Handled = false;
        bool End_Handled = false;
        bool Emblace_erase_combo = false;
        bool EmplacePrevEnd = false;
        bool EmplaceCurrentEnd = false;
        bool erased = false;

        int workingstate = 0;
        int prevworkingstate = workingstate;

        /*!
            @brief Construct a new interval map object
            
            @tparam V_forward Type of the stored values.
            @param val Default Kontent
        */
        interval_map(std::map<K,V> &m_map, V &m_valBegin) : m_map(m_map), m_valBegin(m_valBegin) {};

        /*!
            @brief Enqueues a new interval assignment request for deferred, non-blocking SPS execution.

            @details **Betrachtung:** 
                        Um den SPS-Zyklus vor unvorhersehbaren Laufzeiten zu schützen, werden direkte 
                        Zuweisungen nicht sofort im selben Zyklus im Baum berechnet, sondern in einer 
                        internen Warteschlange (`assignqueue`) gepuffert. Die eigentliche Abarbeitung 
                        erfolgt schrittweise über die State Machine.

            @details **Regeln:** 
                        - Das Intervall schließt `keyBegin` ein und exkludiert `keyEnd`.
                        - Ist das Intervall ungültig (`!(keyBegin < keyEnd)`), wird die Anfrage direkt verworfen.
                        - Dank `std::forward` und `requires`-Constraint wird sichergestellt, dass nur kompatible 
                        Werttypen ohne unnötige Kopien übergeben werden.

            @details **Erkenntnisse:** 
                        Das Abfangen über einen `try-catch`-Block schützt vor unerwarteten Ausnahmen bei 
                        eigenen Key-Vergleichsoperatoren. Das Caching in der Queue garantiert einen deterministischen 
                        Einstieg für den `run()`-Zyklus.

            @tparam V_forawrd Forwarding reference type for the value.
            @param keyBegin Start of the interval (inclusive).
            @param keyEnd End of the interval (exclusive).
            @param val The value to assign across the interval.
        */
        template<typename V_forawrd>
        void assign (K const &keyBegin, K const &keyEnd, V_forawrd &&val) requires (std::is_same<std::remove_cvref_t<V_forawrd>, V>::value)
        {
            try
            {
                if (!(keyBegin < keyEnd)) return;                                               // Pre-exit: ignore invalid interval
            }
            catch(const std::exception& e)
            {
                if(Debugging) std::cout << e.what() << '\n';
                return;
            }
            catch(...)
            {
                if(Debugging) std::cout << "Undefined Error." << '\n';
                return;
            }
            
            INTERVALL Buffer(INTERVALLKEYS(keyBegin, keyEnd),std::forward<V_forawrd>(val));

            assignqueue.push(Buffer);
        };

        /*!
            @brief Compares two keys for equivalence using strict weak ordering rules.

            @details **Betrachtung:** 
                        Da generische Key-Typen in Maps oft nur einen `operator<` und keinen 
                        direkten `operator==` garantieren, wird die Äquivalenz logisch über die 
                        Nicht-Größer-Beziehung in beide Richtungen ermittelt.

            @details **Regeln:** 
                        - Zwei Schlüssel gelten als äquivalent, wenn weder der erste kleiner als der zweite 
                        noch der zweite kleiner als der erste ist.
                        - Vermeidet die direkte Abhängigkeit von einem spezifischen `operator==` der Key-Klasse.

            @details **Erkenntnisse:** 
                        Dieses Standard-Idiom funktioniert universell und robust für alle 
                        Vergleichstypen, selbst wenn der Key-Typ keinen direkten Gleichheitsoperator anbietet.

            @param keyFokus The primary focus key to compare against.
            @param keyRequest The key requested for comparison.

            @return True if both keys are equivalent, false otherwise.
        */
        bool EqualKey(K const &keyFokus, K const &keyRequest)
        {
            if(!(keyRequest < keyFokus) && !(keyFokus < keyRequest))
            {
                return true;
            }
            return false;
        };

        /*!
            @brief Prepares the execution path by inspecting the right boundary of the interval map.

            @details **Betrachtung:** 
                        Zu Beginn eines neuen Auftrags prüft dieser Zustand, ob der neue Bereich 
                        komplett hinter dem letzten Element der bestehenden Map liegt. Dadurch können 
                        unnötige Durchläufe vermieden und direkt optimierte Pfade gewählt werden.

            @details **Regeln:** 
                        - Ist die Map leer oder liegt der neue Startschlüssel hinter dem letzten Schlüssel, 
                        wird direkt in den Emplace-Zustand verzweigt (`direkt_both = true`).
                        - Andernfalls wird die reguläre Initialisierung gestartet, um den Baum von vorne 
                        zu durchlaufen.

            @details **Erkenntnisse:** 
                        Diese Vorab-Prüfung ("Precheck") fängt die häufigsten append-artigen Zuweisungen 
                        frühzeitig ab und spart wertvolle Rechenzeit im SPS-Zyklus.
        */
        inline void State_Precheck()
        {
            prevworkingstate = 0;

            // Preparation check on the rightmost element
            Searchsectionreverse = m_map.rbegin();

            if(Searchsectionreverse != m_map.rend())
            {
                if(Searchsectionreverse->first < assignqueue.front().first.first)
                {
                    direkt_both = true;
                    workingstate = 5;
                }
                else
                {
                    direkt_both = false;
                    workingstate = 1;
                }
            }
            else
            {
                direkt_both = true;
                workingstate = 5;
            }
        };

        /*!
            @brief Initializes the internal iterators, pointers, and flags for a full map traversal.

            @details **Betrachtung:** 
                        Wenn der Precheck ergibt, dass ein vollständiger Durchlauf des Baums notwendig 
                        ist, bereitet dieser Zustand alle Startvariablen vor. Die Iteratoren werden auf den 
                        Anfang gesetzt und die Zeiger auf den aktuellen Auftrag in der Queue gerichtet.

            @details **Regeln:** 
                        - Setzt `Searchsection` und `Searchprev` auf den Beginn der Map.
                        - Holt die Referenzen auf den Fokus-Schlüssel und den Zielwert direkt aus dem 
                        vordersten Element der `assignqueue`.
                        - Setzt Kontroll-Flags (`END`, `EndHandled`) auf den Standardzustand zurück und 
                        schaltet in den Suchzustand (State 2) um.

            @details **Erkenntnisse:** 
                        Durch die klare Trennung von Initialisierung und Suche wird sichergestellt, dass 
                        jeder neue Auftrag mit sauberen und definierten Zuständen startet, ohne Altlasten 
                        aus vorherigen Durchläufen mitzuschleppen.
        */
        inline void State_Init()
        {
            prevworkingstate = 1;

            Searchsection = m_map.begin();
            Searchprev = m_map.begin();

            FokusKey = &assignqueue.front().first.first;
            val = &assignqueue.front().second;
            END = false;
            EndHandled = false;

            workingstate = 2;
        };

        /*!
            @brief Core search and traversal state for analyzing and modifying interval boundaries.

            @details **Betrachtung:** 
                        Dieser Zustand bildet das Herzstück der State Machine. Er iteriert schrittweise 
                        durch den Baum, vergleicht Schlüssel und entscheidet, ob bestehende Intervalle 
                        angepasst, Werte überschrieben oder Bereiche gelöscht/neu eingefügt werden müssen.

            @details **Regeln:** 
                        - Arbeitet inkrementell, um den SPS-Zyklus nicht durch lange Schleifen zu blockieren.
                        - Unterscheidet präzise zwischen der Bearbeitung des Start-Schlüssels (`!END`) 
                        und des End-Schlüssels (`END`).
                        - Steuert die Übergänge zu den Emplace- und Erase-Zuständen je nach Wertekonsistenz.

            @details **Erkenntnisse:** 
                        Die Aufteilung in feingranulare Vergleiche verhindert teure Mehrfach-Suchen und 
                        garantiert, dass jede Anpassung im Baum exakt an der richtigen Position greift.
        */
        inline void State_Search ()
        {      
            prevworkingstate = 2;

            if(Searchsection == m_map.end()) 
            {
                if(!End_Handled)    workingstate = 6;                               // Edge case: Reached map end without handling end key -> force emplace end
                else                workingstate = 3;                               // Finished -> proceed to canonicalize
                return;
            }
            
            bool Skip = false;
            
            Searchnext = std::next(Searchsection);

            if(Searchnext != m_map.end())
            {
                if(*FokusKey < Searchnext->first)                                   // Condition whether the successor key is smaller than current focus
                {
                    if(EqualKey(*FokusKey, Searchsection->first))                   // We are precisely on the key
                    {
                        if(!END)
                        {
                            if(!(Searchprev->second == *val))
                            {
                                Searchsection->second = *val;                       // Edge case: Exact key match at begin -> overwrite value in-place
                            }
                            
                            FokusKey = &assignqueue.front().first.second;
                            END = true;
                            workingstate = 8; return;
                        }
                        else                                                        // Landing on keyEnd and values are identical -> cleanup and return
                        {                        
                            if(Searchsection->second == *val) 
                            { 
                                workingstate = 7;                                   // Edge case: Value matches boundary -> erase redundant node
                                End_Handled = true;
                                return; 
                            }
                            
                            workingstate = 4; return;
                        }
                    }
                    else                                                            // We are located between intervals
                    {
                        bool SeachCurrentEqual = Searchsection->second == *val;

                        bool Search_Smaller_Ley = *FokusKey < Searchsection->first;
                        if(!END)
                        {
                            if(!Begin_Handled &&(Search_Smaller_Ley || !(SeachCurrentEqual)))
                            { 
                                workingstate = 5;                                   // Edge case: Insert new begin before current section
                                
                                return;
                            }   
                            else if(!Begin_Handled && SeachCurrentEqual)
                            {
                                
                                END = true;
                                Begin_Handled = true;
                                FokusKey = &assignqueue.front().first.second;
                                if( Searchnext->first < *FokusKey)
                                {
                                    workingstate = 8;                               // Edge case: Merged value spans across next node -> advance iterator
                                }
                                else 
                                    workingstate = 2;

                                return;
                            }                                                                                                                       
                        }
                        else 
                        {
                            if(!End_Handled)
                            {
                                if(Search_Smaller_Ley && !(Searchprev->second == *val))
                                {                           
                                    EmplacePrevEnd = true;
                                    if(Searchprev->second == Searchsection->second)
                                    {                                
                                        erase_prev = true;
                                        Emblace_erase_combo = true;         // Edge case: Merge adjacent identical values during end insertion
                                    }
                                    workingstate = 6; return;
                                }
                                else if((SeachCurrentEqual) && (m_map.begin() != Searchsection))
                                {
                                    if(std::prev(Searchsection)->second == *val)
                                    {
                                        workingstate = 7;                   // Edge case: Redundant node detected during end phase -> remove it
                                        End_Handled = true;
                                        return;
                                    }
                                        
                                }    
                                else if(!(SeachCurrentEqual) && (!(Searchprev->second == *val) || (Searchsection->first < *FokusKey)))
                                {
                                    EmplaceCurrentEnd = true;               // m_map.emplace_hint(Searchnext, FokusKey, Searchsection->second);
                                                                            // Jump to (Default) m_map.erase(Searchsection);
                                    Emblace_erase_combo = true;             // Jump from emplace to Erase

                                    workingstate = 6; return;               // Starting the Transversion to emplace
                                }   
                            }                                                                               
                            
                            workingstate = 3; return;
                        }
                    }                        
                }
                else
                {
                    if(END)                                                 // Deletion is only allowed once we have passed the keyBegin boundary
                    {
                        workingstate = 7; 

                        return;                                             // Edge case: Clean up covered nodes within the interval range
                    }
                }                     
            }
            else
            {                    
                if(*FokusKey < Searchsection->first)
                {
                    EmplacePrevEnd = true;
                    direkt_both = true;
                    workingstate = 6; return;                           // Edge case: Insert before last element when key is smaller
                }
                if(END)
                {                
                    workingstate = 7; return;                           // Edge case: Final sweep deletion at map tail
                }
            }

            workingstate = 8;
        
            if(!EndHandled && Begin_Handled)
            {
                direkt_both = true;
                workingstate = 6; return;                               // Edge case: Finalize trailing end placement
            }
        };

        /*!
            @brief Canonicalizes the interval map by removing redundant tail entries where the last two values are identical.

            @details **Betrachtung:** 
                        Durch komplexe Intervall-Änderungen kann es vorkommen, dass am Ende der Map 
                        ein Element entsteht, dessen Wert exakt dem Wert des direkten Vorgängers 
                        entspricht. Das verletzt die Regel, dass benachbarte Intervalle unterschiedliche 
                        Werte aufweisen müssen.

            @details **Regeln:** 
                        - Prüft, ob die Map mindestens zwei Elemente enthält.
                        - Vergleicht die Werte (`.second`) der letzten beiden Elemente (`prevIt->second == lastIt->second`).
                        - Sind die Werte identisch, wird das redundante letzte Element via `m_map.erase(lastIt)` entfernt.

            @details **Erkenntnisse:** 
                        Dieser gezielte Check am Ende hält die Map kompakt und verhindert redundante 
                        Schlüssel-Wert-Paare am Ende des Baums.
        */
        inline void State_Canonicalize()
        {
            if (m_map.size() >= 2)                                      // Final sweep: Check canonicality at the map tail
            {
                auto lastIt = std::prev(m_map.end());
                auto prevIt = std::prev(lastIt);

                                                                        // If the values of the last two elements are identical,
                                                                        // the tail entry is redundant and violates canonicality.
                if (prevIt->second == lastIt->second)
                {
                    m_map.erase(lastIt);                                // Remove redundant tail node
                }
            }
            
            prevworkingstate = 3;
            workingstate = 4;
        };

        /*!
            @brief Cleans up after assignment completion, resets state flags, and pops the processed queue item.

            @details **Betrachtung:** 
                        Dieser Zustand schließt den aktuellen Zuweisungsauftrag ab. Er räumt alle 
                        während der State-Machine-Durchläufe gesetzten Kontroll-Flags auf, bereinigt 
                        eventuelle Einzel-Einträge und entfernt den verarbeiteten Auftrag aus der Queue.

            @details **Regeln:** 
                        - Hat die Map nach der Bearbeitung genau eine Größe von 1, wird sie komplett geleert (`m_map.clear()`).
                        - Setzt alle Steuer-Flags (`END`, `Begin_Handled`, `direkt_both`, etc.) vollständig auf `false` zurück.
                        - Entfernt das fertige Element via `assignqueue.pop()` und springt zurück in den Startzustand (`workingstate = 0`) für den nächsten Auftrag.

            @details **Erkenntnisse:** 
                        Ein deterministischer Cleanup ist essenziell, um zu verhindern, dass alte 
                        Steuer-Flags in den nächsten SPS-Zyklus verschleppt werden und dort zu Fehlverhalten führen.
        */
        inline void State_Cleanup()
        {
            prevworkingstate = 4;

            if(m_map.size() == 1) m_map.clear();            // Reset map if only a single default element remains

            // Reset all control and tracking flags for the next run
            END = false;
            EndHandled = false;
            direkt_both = false;
            erase_prev = false;
            Begin_Handled = false;
            End_Handled = false;
            Emblace_erase_combo = false;
            EmplacePrevEnd = false;
            EmplaceCurrentEnd = false;
            erased = false;

            assignqueue.pop();                              // Remove completed job from the queue
            workingstate = 0;                               // Return to initial state for subsequent jobs
        };

        /*!
            @brief Inserts the interval's start key into the map using optimized hint positions.

            @details **Betrachtung:** 
                        Dieser Zustand kümmert sich um das gezielte Einfügen des Startschlüssels (`keyBegin`) 
                        in die bestehende Map. Je nach vorherigem Zustand geschieht dies über einen berechneten 
                        Hint-Iterator (`emplace_hint`) oder direkt am Ende der Map.

            @details **Regeln:** 
                        - Kommt der Aufruf aus der Suche (`prevworkingstate == 2`), wird geprüft, ob vor oder nach 
                        dem aktuellen Sektorelement eingefügt werden muss.
                        - Setzt nach dem Einfügen das `Begin_Handled`-Flag, schaltet auf die End-Bearbeitung um (`END = true`) 
                        und richtet den Fokus auf das `keyEnd` des Auftrags.
                        - Steuert die Folgerichtung (`workingstate = 8`, `6` oder `2`) abhängig davon, ob Sprünge oder 
                        direkte Pfade erforderlich sind.

            @details **Erkenntnisse:** 
                        Die Verwendung von `emplace_hint` in Kombination mit dem korrekten Nachfolger- oder Vorgänger-Iterator 
                        gewährleistet eine performante Einfügeoperation mit minimalem Suchaufwand im Baum.
        */
        inline void Emplace_KeyBegin()
        {
            bool next_next = false;

            if(prevworkingstate == 2)
            {
                if(Searchsection->first < *FokusKey)    
                { 
                    m_map.emplace_hint(Searchnext, *FokusKey, std::forward<V>(*val));  
                    next_next = true;                                   // Edge case: Focus key lies beyond current section -> advance iterator path
                }
                else
                {
                    m_map.emplace_hint(Searchsection, *FokusKey, std::forward<V>(*val));
                }
            }   
            else
            {
                m_map.emplace_hint(m_map.end(), assignqueue.front().first.first, std::forward<V>(assignqueue.front().second));  // Direct append at map tail
            }

            prevworkingstate = 5;
            Begin_Handled = true;
            END = true;

            FokusKey = &assignqueue.front().first.second;               // Switch focus to interval end key

            if(next_next) workingstate = 8;
            else if(direkt_both) workingstate = 6;
            else workingstate = 2;
        };

        /*!
            @brief Inserts the interval's end key into the map using corresponding boundary values.

            @details **Betrachtung:** 
                        Dieser Zustand fügt das Ende eines zugewiesenen Intervalls (`keyEnd`) in die 
                        Map ein. Dabei muss sichergestellt werden, dass der ursprüngliche Zustand 
                        außerhalb des neuen Intervalls korrekt wiederhergestellt wird (entweder durch den 
                        vorherigen Wert, den aktuellen Sektorwert oder den Standardwert `m_valBegin`).

            @details **Regeln:** 
                        - Kommt der Aufruf aus der Suche (`prevworkingstate == 2`), wird anhand von Flags 
                        (`EmplaceCurrentEnd`, `EmplacePrevEnd`) entschieden, welcher Wert am Endpunkt platziert wird.
                        - Andernfalls wird der Endpunkt direkt am Ende der Map mit dem Standardwert eingefügt.
                        - Setzt das `End_Handled`-Flag und steuert je nach Kombi-Flags (`Emblace_erase_combo`, `direkt_both`) 
                        den Übergang zum nächsten Zustand (Löschen, Kanonisieren oder Weitersuchen).

            @details **Erkenntnisse:** 
                        Die differenzierte Wahl des Hint-Iterators und des Wertes garantiert, dass die 
                        Intervallgrenzen lückenlos und ohne unerwünschte Werte Sprünge in die Map geschrieben werden.
        */
        inline void Emplace_KeyEnd()
        {
            
            if(prevworkingstate == 2)
            {
                if(EmplaceCurrentEnd)   m_map.emplace_hint(Searchnext, *FokusKey, Searchsection->second);                   // Edge case: Restore current section value at end boundary
                else if(EmplacePrevEnd) m_map.emplace_hint(Searchsection, *FokusKey, Searchprev->second);                   // Edge case: Restore previous section value at end boundary
                else                    m_map.emplace_hint(Searchsection, *FokusKey, m_valBegin);                           // Fallback: Restore default value at end boundary
            }
            else                        m_map.emplace_hint(m_map.end(), assignqueue.front().first.second, m_valBegin);      // Direct tail append with default value

            End_Handled = true;

            prevworkingstate = 6;
            if(Emblace_erase_combo) workingstate = 7;                                                                       // Transition to erase if combined with a cleanup action
            else if(direkt_both) workingstate = 3;                                                                          // Transition to canonicalize on direct paths
            else workingstate = 2;                                                                                          // Continue search loop otherwise
        };

        /*!
            @brief Erases the current or previous map section node during interval reconstruction.

            @details **Betrachtung:** 
                        Während des Intervall-Mappings müssen überschriebene oder redundant gewordene 
                        Zwischenknoten aus der Map entfernt werden. Dieser Zustand steuert das sichere 
                        Löschen über die entsprechenden Iteratoren (`Searchsection` oder `Searchprev`).

            @details **Regeln:** 
                        - Ist das `erase_prev`-Flag gesetzt, wird der vorherige Knoten (`Searchprev`) gelöscht, 
                        andernfalls der aktuelle Sektor (`Searchsection`).
                        - Setzt das `erased`-Flag, sobald ein Knoten erfolgreich entfernt wurde, um nachfolgende 
                        Iterator-Sprünge korrekt zu steuern.
                        - Steuert die Folgerichtung (`workingstate = 3`, `8` oder `2`) basierend auf dem Herkunftszustand 
                        und dem Löschstatus.

            @details **Erkenntnisse:** 
                        Durch das gezielte Löschen einzelner überlappender Knoten wird sichergestellt, 
                        dass der Baum nach dem Einfügen des neuen Intervalls sauber und ohne veraltete 
                        Zwischenwerte bleibt.
        */
        inline void Erase_Current()
        {
            if(Searchprev == Searchsection) erased = true;
            if(erase_prev) 
            {
                m_map.erase(Searchprev);                // Edge case: Remove previous redundant node
            }       
            else 
            {
                m_map.erase(Searchsection);
                erased = true;                          // Edge case: Remove current covered node and flag deletion
            }       

            if(prevworkingstate == 6) workingstate = 3; // Transition to canonicalize after end combination
            else if(erased)  workingstate = 8;          // Advance via Set_Next if deletion occurred
            else        workingstate = 2;               // Continue search loop otherwise
            prevworkingstate = 7;
        };

        /*!
            @brief Advances the internal traversal iterators after a search or erase step.

            @details **Betrachtung:** 
                        Damit die State Machine den Baum schrittweise durchlaufen kann, müssen die 
                        aktiven und vorherigen Iteratoren (`Searchsection`, `Searchprev`, `Searchnext`) 
                        nach jedem Verarbeitungs- oder Löschschritt korrekt aktualisiert werden.

            @details **Regeln:** 
                        - War ein Löschvorgang aktiv (`erased = true`), wird `Searchprev` auf den nächsten 
                        Sektor (`Searchnext`) gesetzt, da der aktuelle Knoten weggefallen ist.
                        - Andernfalls rückt `Searchprev` auf das bisherige `Searchsection` vor.
                        - `Searchsection` übernimmt den Wert von `Searchnext`, setzt das `erased`-Flag 
                        wieder zurück und springt zurück in den Suchzustand (`workingstate = 2`).

            @details **Erkenntnisse:** 
                        Diese saubere Iterator-Nachführung verhindert Iterator-Invalidierungen und 
                        garantiert, dass der fortlaufende Durchlauf des Baums reibungslos funktioniert.
        */
        inline void Set_Next()
        {
            prevworkingstate = 8;

            if(erased) Searchprev = Searchnext;     // Edge case: Node was erased, skip prev pointer to next
            else Searchprev = Searchsection;        // Standard advance: prev catches up to current section

            Searchsection = Searchnext;  
            erased = false;     

            workingstate = 2;                       // Return to search state for the next iteration
        };

        /*!
            @brief Checks whether the assignment queue contains pending tasks for the state machine.

            @details **Betrachtung:** 
                        Um zu steuern, ob der SPS-Zyklus Verarbeitungszeit für das Intervall-Mapping 
                        aufwenden muss, prüft diese Methode, ob Aufträge in der Warteschlange liegen.

            @details **Regeln:** 
                        - Gibt `true` zurück, wenn `assignqueue` nicht leer ist (`!assignqueue.empty()`).
                        - Gibt `false` zurück, wenn aktuell keine ausstehenden Zuweisungen existieren.

            @details **Erkenntnisse:** 
                        Eine minimale, non-blocking Abfrage, die als Guard für den Einstieg in die 
                        State-Machine-Zyklen dient.

            @return True if pending tasks exist in the queue, false otherwise.
        */
        bool filled_queue()
        {
            if(!assignqueue.empty()) return true; // Queue has pending assignments
            return false;
        };

        /*!
            @brief Executes a single non-blocking step of the state machine for active interval assignments.

            @details **Betrachtung:** 
                        Um die Echtzeitanforderungen und den Jitter-freien Betrieb in SPS-Umgebungen 
                        zu gewährleisten, verteilt diese Methode die Berechnungen auf feingranulare Zyklen. 
                        Anstatt den gesamten Baum in einem Durchlauf zu blockieren, wird pro Aufruf exakt 
                        ein Zustand der State Machine abgearbeitet.

            @details **Regeln:** 
                        - Ist die Warteschlange leer, wird sofort mit `0` abgebrochen.
                        - Steuert über eine `switch`-Anweisung basierend auf `workingstate` (0 bis 8) 
                        den jeweiligen Funktionsaufruf (`State_Precheck`, `State_Search`, etc.).
                        - Gibt den aktuellen `workingstate` als Rückgabewert zurück, um den Fortschritt 
                        nach außen oder an übergeordnete Task-Schleifen zu signalisieren.

            @details **Erkenntnisse:** 
                        Dieses zustandsbasierte Dispatching garantiert konstante, extrem kurze Ausführungszeiten 
                        pro SPS-Zyklus (~100ns im Durchschnitt) und verhindert unzulässige Latenzspitzen.

            @return The current working state index after execution, or 0 if the queue is empty.
        */
        int run()
        {
            if(assignqueue.empty()) return 0; // Pre-exit: Nothing to process in the queue

            switch (workingstate)
            {
            case 0:
                State_Precheck();
                break;
            case 1:
                State_Init();
                break;
            case 2:
                State_Search();
                break;
            case 3:
                State_Canonicalize();
                break;
            case 4:
                State_Cleanup();
                break;
            case 5:
                Emplace_KeyBegin();
                break;
            case 6:
                Emplace_KeyEnd();
                break;
            case 7:
                Erase_Current();
                break;
            case 8:
                Set_Next();
                break;
            
            default:
                break;
            }

            return workingstate;
        };

        /*!
            @brief  look-up of the value associated with key   
        */
        V const &operator[](K const &key) const
        {
            auto it = m_map.upper_bound(key);
            if(it == m_map.begin())
            {
                return m_valBegin;
            }
            else
            {
                return (--it)->second;
            }
        };
    };
};
