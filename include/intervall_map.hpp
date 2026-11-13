#pragma once
/*!
    @file intervall_map.hpp
    @author your name (you@domain.com)
    @brief 
    @version 0.1
    @date 2026-11-13

    @copyright Copyright (c) 2026
*/
#include <map>
#include <chrono>

namespace PC
{
    /*!
        @brief Host-side PC wrapper and test harness for the interval map, optimized for interactive visualization, benchmarking, and validation.

        @details **Betrachtung:** 
                    Diese Klasse/Struktur stellt das PC-seitige Pendant zur SPS-Logik dar. 
                    Sie dient der interaktiven Visualisierung (via Dear ImGui), dem Durchführen 
                    umfangreicher Testmatrizen (`Testcase1`) sowie der automatisierten Validierung 
                    gegen eine Referenz-Implementation (`std::map`).

        @details **Regeln:** 
                    - Erlaubt kontrollierten Einblick in interne Zustände (z.B. via `friend` oder `public`) für Test- und Debugging-Zwecke.
                    - Erfasst präzise Timing-Diagnostiken im Nanosekundenbereich.
                    - Entkoppelt die Visualisierung und Testläufe von den harten Echtzeit-Restriktionen des Zielsystems.

        @details **Erkenntnisse:** 
                    Die Trennung in PC- und SPS-Namespace erlaubt es, komplexe Zustandsübergänge 
                    und Performance-Vergleiche transparent auf dem Host zu evaluieren, bevor der 
                    Code auf die Ziel-SPS portiert wird.

        @tparam K Type of the key elements.
        @tparam V Type of the stored values.
    */
    template <typename K, typename V>
    class interval_map
    {
        friend void IntervalMapTest();        
        bool Debugging = false;
        

    public:
        std::map<K,V> m_map;
        V m_valBegin;

        int assignselection = 2;

        //Consturctor associates whole range of K with val
        template<typename V_forward>
        interval_map(V_forward &&val) : m_valBegin(std::forward<V_forward>(val)) {};

        /*!
            @brief Assigns a value to a given half-open interval [keyBegin, keyEnd], overwriting previous values.

            @tparam V_forward Forwarding type for the value, constrained to match value type V.
            @param keyBegin Start key of the interval (inclusive).
            @param keyEnd End key of the interval (exclusive).
            @param val The value to assign across the interval.

            @details **Betrachtung:** 
                        Die `assign`-Methode ist der Einstiegspunkt für die Zuweisung von Intervallen. 
                        Da Intervalle in unterschiedlichen Bereichen des bestehenden Baums liegen können 
                        (relativ zu vorhandenen Grenzen und Standardwerten), steuert eine Matrix aus 8 
                        Kombinationsszenarien die genaue Strategie zur Aufteilung und Bereinigung.

            @details **Regeln:** 
                        - Entspricht das Intervall der Bedingung `!(keyBegin < keyEnd)`, wird es als ungültig 
                        bzw. leer betrachtet und die Operation sofort abgebrochen (Pre-Exit).
                        - Zustandsklassifizierung basiert auf drei Kriterien:
                        - (A/a) Wert ist Default vs. Wert ist nicht Default
                        - (B/b) KeyEnd liegt hinter vs. vor dem letzten Eintrag
                        - (C/c) KeyBegin liegt hinter vs. vor dem ersten Eintrag
                        - Kombinationen und Lösungsstrategien (1 bis 8):
                        - (1) **ABC**: KeyBegin einfügen -> Löschung ab KeyBegin-Next bis End
                        - (2) **AbC**: KeyEnd erstellen (mit prev Value) -> Löschen von Beginn bis KeyEnd -> KeyBegin einfügen
                        - (3) **ABc**: Löschen der gesamten Map
                        - (4) **Abc**: KeyEnd erstellen (mit prev Value) -> Löschen von Beginn bis KeyEnd
                        - (5) **aBC**: KeyBegin einfügen -> Löschen von KeyBegin bis End -> KeyBegin & KeyEnd einfügen
                        - (6) **abC**: KeyEnd einfügen -> KeyBegin einfügen -> Löschen von KeyBegin-Next bis KeyEnd
                        - (7) **aBc**: Map löschen -> KeyBegin einfügen -> KeyEnd einfügen
                        - (8) **abc**: KeyEnd einfügen -> KeyBegin einfügen -> Löschen von KeyBegin-Next bis KeyEnd

            @details **Erkenntnisse:** 
                        Eine saubere Vorab-Klassifizierung verhindert redundante Baum-Operationen und 
                        stellt sicher, dass die Intervallgrenzen in SPS-Umgebungen deterministisch und fehlerfrei 
                        angepasst werden.
        */
        template<typename V_forward>
        void assign (K const &keyBegin, K const &keyEnd, V_forward &&val) requires (std::is_same<std::remove_cvref_t<V_forward>, V>::value)
        {
            try
            {
                if (!(keyBegin < keyEnd)) return;                                     // PreExit: Invalid interval bounds, do nothing
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
            
            
            switch (assignselection)
            {
            case 1:
                assignv1(keyBegin, keyEnd, std::forward<V_forward>(val));
                break;
            case 2:
                assignv2(keyBegin, keyEnd, std::forward<V_forward>(val));
                break;
            case 3:
                assignv3(keyBegin, keyEnd, std::forward<V_forward>(val));
                break;
            case 4:
                assignv4(keyBegin, keyEnd, std::forward<V_forward>(val));
                break;
            
            default:
                if(Debugging) std::cout << "Nice try, the Assign doesnt exist" << std::endl;
                break;
            }        
        };

        /*!
            @brief First version focusing strictly on boundary handling and strong exception safety.

            @tparam V_forward Forwarding type for the value, constrained to match value type V.
            @param keyBegin Start key of the interval (inclusive).
            @param keyEnd End key of the interval (exclusive).
            @param val The value to assign across the interval.

            @details **Betrachtung:** 
                        Diese Implementierung (`assignv1`) fokussiert sich primär auf die exakte Behandlung 
                        von Intervallgrenzen. Sie ist als robuster Prototyp ausgelegt, der die starke 
                        Exception-Sicherheit (*Strong Exception Guarantee*) gewährleistet, indem temporäre 
                        Paare vor jeglicher Modifikation der Haupt-Map erzeugt werden.

            @details **Regeln:** 
                        - Erstellung von Start- und End-Paaren vor dem Einfügen, um bei Kopierfehlern den Zustand 
                        der Map unberührt zu lassen.
                        - Nutzung von `std::next` und `std::prev` zur Ermittlung des gültigen Bereinigungsbereichs (`erase range`).
                        - Durchführung eines abschließenden Bereinigungsschritts (*"Besenstrich"*), um redundante 
                        Dopplungen am Ende der Map zu eliminieren.

            @details **Erkenntnisse:** 
                        Obwohl hier bewusst vom strikten logN-Einzelpfad abgewichen wird, um die Grenzlogik 
                        klar zu isolieren, bietet dieser Ansatz eine hervorragende Basis zur Validierung 
                        der Korrektheit von Intervallgrenzen.
        */
        template<typename V_forward>
        void assignv1 (K const &keyBegin, K const &keyEnd, V_forward &&val)
        {
            //--------------------------------------------------------------Setup temporary variables
            typename std::map<K, V>::iterator tempbeginIT;
            bool insertedstart = false;

            typename std::map<K, V>::iterator tempendIT;
            bool insertedend = false;

            //--------------------------------------------------------------Create pairs safely (Strong Exception Guarantee)
            try
            {
                std::pair<K,V> tempbegin = std::pair<K,V>(keyBegin, std::forward<V_forward>(val));                                      // Try creating start pair; aborts if K or V copy fails
                std::pair<K,V> tempend = std::pair<K,V>(keyEnd, m_valBegin);                                                            // Try creating end pair with default value
            
                //--------------------------------------------------------------Insert start boundary
                auto [tempbeginIT_mem, insertedstart_mem] = m_map.insert(tempbegin);                                                                 // Insert start key and store iterator + insert status
                tempbeginIT = tempbeginIT_mem;
                insertedstart = insertedstart_mem;


                if(std::next(tempbeginIT) != m_map.end())                                                                               // Check if there is a next element
                {
                    if(keyEnd < std::next(tempbeginIT)->first)                                                                          // Check if interval falls inside a single existing segment
                    {
                        if(!insertedstart && tempbeginIT->second == val)                                                                // Check if key exists and value is already identical
                        {
                            return;                                                                                                     // Exit early since no change is needed
                        }
                        tempend.second = std::prev(tempbeginIT)->second;                                                                // Use previous interval value for end key
                    }
                }

                auto [tempendIT_mem, insertedend_mem] = m_map.insert(tempend);                                                                       // Insert end key to set right boundary
                tempendIT = tempendIT_mem;
                insertedend = insertedend_mem;
            }
            catch(const std::exception& e)                                                                                              // Catch object construction errors
            {
                if(Debugging) std::cerr << e.what() << '\n';                                                                            // Optional Print debug error
                return;                                                                                                                 // Do nothing on failure (map stays untouched)
            }
            catch(...)
            {
                if(Debugging) std::cerr << "Undefined Error Handling." << std::endl;
                return;
            }

            //--------------------------------------------------------------Set up range to erase
            auto DeleteStart = tempbeginIT;                                                                                             // Mark start of erase range
            auto DeleteEnd = tempendIT;                                                                                                 // Mark end of erase range

            //--------------------------------------------------------------Fix left boundary
            if(!insertedstart && !(tempbeginIT->second == val))                                                                         // Check if start key existed with a different value
            {
                tempbeginIT->second = val;                                                                                              // Update value at existing key
                DeleteStart = std::next(tempbeginIT);                                                                                   // Keep start key, erase starting from next element
            }
            else
            {
                DeleteStart = std::next(tempbeginIT);                                                                                   // Start erasing right after start key
            }
            
            if(tempbeginIT != m_map.begin())                                                                                            // Check if a previous element exists
            {
                auto fortempbeginit = std::prev(tempbeginIT);                                                                           // Get previous iterator

                if(!(fortempbeginit->second == tempbeginIT->second))                                                                    // Check if previous value is different
                {
                    DeleteStart = std::next(tempbeginIT);                                                                               // Keep key since value changed
                }
                else
                {
                    DeleteStart = tempbeginIT;                                                                                          // Same value as previous; include start key in erase range
                }
            }

            //--------------------------------------------------------------Fix right boundary
            if(DeleteEnd != DeleteStart)                                                                                                // Check if there is a valid range to clean up
            {
                if(insertedend)                                                                                                         // Check if end key was newly inserted
                {
                    if(!(tempbeginIT->second == std::prev(tempendIT)->second))                                                          // Check if value changes before end key
                        tempendIT->second = std::prev(tempendIT)->second;                                                               // Copy value from key before it
                    else
                    {
                        auto followit = std::next(tempendIT);                                                                           // Get element after end key
                        if(followit != m_map.end())                                                                                     // Stay within map bounds
                        {
                            DeleteEnd = followit;                                                                                       // Expand erase range to next element
                        }
                    }
                }

                auto tempbehindendIT = std::next(tempendIT);                                                                                                        // Get element right after end key
                if(tempbehindendIT != m_map.end())                                                                                                                  // Stay within map bounds
                {
                    // Check for duplicate values at the end
                    if(tempendIT->second == tempbehindendIT->second || (!insertedend && DeleteEnd->second == DeleteStart->second) && (DeleteStart->second == val))  // Check for duplicate values at the end
                    {
                        DeleteEnd = tempbehindendIT;                                                                                                                // Include duplicate end key in erase range
                    }
                }
            }
            else
            {
                if(DeleteEnd != m_map.end())
                {
                    auto buffer = std::next(DeleteEnd);
                    auto buffer2 = std::prev(DeleteEnd);
                    if(buffer != m_map.end())
                    {
                        if(buffer->second == DeleteEnd->second || buffer2->second  == DeleteEnd->second)
                        {
                            DeleteEnd = buffer;
                        }
                    }
                }                
            }

            //--------------------------------------------------------------Erase duplicate range and clean up
            if(DeleteStart != DeleteEnd) 
                m_map.erase(DeleteStart, DeleteEnd);                                                                                       // Erase elements in range
            
            // Final cleanup at the map tail
            if (m_map.size() >= 2)                                                                                                                                  // Check if map has at least 2 elements
            {
                auto lastIt = std::prev(m_map.end());                                                                                                               // Get last element
                auto prevIt = std::prev(lastIt);                                                                                                                    // Get second-to-last element

                if (prevIt->second == lastIt->second)                                                                                                               // Check if last two elements have same value
                {
                    m_map.erase(lastIt);                                                                                                                            // Erase redundant last element
                }
            }

            if(m_map.size() == 1) m_map.clear();                                                                                                                    // Clear map if only default value remains
        };

        /*!
            @brief Optimized interval assignment using lower_bound and sequential iterator navigation.

            @tparam V_forward Forwarding type for the value, constrained to match value type V.
            @param keyBegin Start key of the interval (inclusive).
            @param keyEnd End key of the interval (exclusive).
            @param val The value to assign across the interval.

            @details **Betrachtung:** 
                        `assignv2` optimiert den Zuweisungsprozess gegenüber `assignv1`, indem ein initialer 
                        `lower_bound` als Sprungmarke genutzt wird. Dadurch werden teure Baum-Traversierungen 
                        minimiert und sequenzielles Iterator-Jumping ermöglicht.

            @details **Regeln:** 
                        - Nutzt `emplace_hint` für zielgerichtete Einfügeoperationen im direkten Iterator-Umfeld.
                        - Zur Erfüllung der Strong Exception Guarantee werden modifizierte oder überschriebene Knoten 
                        vorab mittels `m_map.extract()` in einer temporären Hilfs-Map gesichert.
                        - Tritt während der Operation eine Exception auf, wird der ursprüngliche Zustand 
                        vollständig über `m_map.merge(temp)` zurückgerollt (*Rollback*).

            @details **Erkenntnisse:** 
                        Diese Variante schlägt die Brücke zwischen hoher Performance (durch lokale Hints) und 
                        ausgezeichneter Ausfallsicherheit (Exception Safety), ideal für deterministische Abläufe.
        */
        template<typename V_forward>
        void assignv2 (K const &keyBegin, K const &keyEnd, V_forward &&val)
        {
            //--------------------------------------------------------------Setup local variables
            auto itBegin = m_map.lower_bound(keyBegin);                                                                                 // Find first iterator greater than or equal to keyBegin
            bool endforend = false;                                                                                                     // Track if end boundary was handled
            std::map<K,V> temp;                                                                                                         // Hold extracted nodes for exception rollback
            bool skip_keybegin = false;                                                                                                 // Track if start key handling can be skipped

            try
            {
                //--------------------------------------------------------------Right Side Behaivior
                if(itBegin == m_map.end())
                {
                    auto itKeyBegin = m_map.emplace_hint(m_map.end(), keyBegin, std::forward<V_forward>(val));                          // Insert start key at map end
                    auto itKeyEnd = m_map.emplace_hint(itKeyBegin, keyEnd, m_valBegin);                                                 // Insert end key with default fallback value
                }
                //--------------------------------------------------------------Left and Middle Side Behaivior
                else
                {
                    auto nextIt = itBegin;                                                                                              // Active pointer for range processing
                    
                    if(itBegin != m_map.begin())                                                                                        // Pre-check predecessor element
                    {
                        auto precheck = std::prev(itBegin);                                                                             // Get direct predecessor
                        if(precheck->second == val)
                        {
                            skip_keybegin = true;
                            auto checkafter = std::next(itBegin);                                                                       // Get successor element
                            
                            if(checkafter != m_map.end())
                            {
                                if(keyEnd < checkafter->first && (keyEnd < itBegin->first))
                                {
                                    return;                                                                                             // Early exit if interval fits inside existing segment
                                }
                            }
                            else
                            {
                                if(keyEnd < itBegin->first)
                                {
                                    return;                                                                                             // Early exit at map tail
                                }
                            }
                            
                            itBegin = precheck;                                                                                         // Shift start pointer back
                        }
                        else
                        {
                            if(keyBegin < itBegin->first)
                            {
                                auto itKeyBegin = m_map.emplace_hint(itBegin, keyBegin, std::forward<V_forward>(val));                  // Insert new start key before current bound
                            }
                            else
                            {
                                auto checkafter = std::next(itBegin);
                                if(!(itBegin->second == val))
                                {
                                    temp.insert(temp.end(), m_map.extract(itBegin));                                                    // Extract mismatched node for rollback
                                    auto itKeyBegin = m_map.emplace_hint(checkafter, keyBegin, std::forward<V_forward>(val));           // Insert corrected start key
                                    nextIt = checkafter;
                                }
                                else
                                {
                                    if(keyEnd < checkafter->first)
                                    {
                                        return;                                                                                         // Early exit if redundant
                                    }
                                    nextIt = checkafter;
                                }
                            }

                            if(keyEnd < nextIt->first)
                            {
                                endforend = true;
                                m_map.emplace_hint(nextIt, keyEnd, precheck->second);                                                   // Insert right boundary and terminate
                                return;
                            }
                        }
                    }
                    else
                    {
                        // Handle edge case at the very beginning of the map
                        if(keyBegin < itBegin->first)
                        {
                            auto itKeyBegin = m_map.emplace_hint(m_map.begin(), keyBegin, std::forward<V_forward>(val));                // Insert at absolute map start
                        }
                        else
                        {
                            auto MemoryIT = std::next(itBegin);                                                                         // Save successor reference
                            temp.insert(temp.end(), m_map.extract(nextIt));                                                             // Extract old boundary node

                            auto itKeyBegin = m_map.emplace_hint(MemoryIT, keyBegin, std::forward<V_forward>(val));                     // Insert new start key
                            nextIt = MemoryIT;
                        }
                    }

                    // Process range and extract nodes for Strong Exception Guarantee
                    while(nextIt != m_map.end())
                    {
                        if(nextIt->first < keyEnd)
                        {
                            auto next_It = std::next(nextIt);                                                                           // Cache successor pointer
                            temp.insert(temp.end(), m_map.extract(nextIt));                                                             // Move node to temp storage
                            nextIt = next_It;                                                                                           // Advance active pointer
                        }
                        else
                        {
                            if(!(nextIt->first < keyEnd) && !(keyEnd < nextIt->first))
                            {
                                if(nextIt->second == val)
                                {
                                    temp.insert(temp.end(), m_map.extract(nextIt));                                                     // Extract redundant exact match
                                }
                                endforend = true;
                            }
                            else
                            {
                                if(!temp.empty())
                                {
                                    endforend = true;
                                    if(!(std::prev(nextIt)->second == temp.rbegin()->second))
                                    {
                                        auto itkeyend = m_map.emplace_hint(nextIt, keyEnd, temp.rbegin()->second);                      // Insert end key with preserved value
                                        auto behindend = std::next(itkeyend);

                                        if(behindend != m_map.end())
                                        {
                                            if(behindend->second == temp.rbegin()->second)
                                            {
                                                m_map.erase(behindend);                                                                 // Prune redundant successor node
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    auto itkeyend = m_map.emplace_hint(nextIt, keyEnd, m_valBegin);
                                    auto behindend = std::next(itkeyend);
                                    endforend = true;

                                    if(behindend != m_map.end())
                                    {
                                        if(behindend->second == itkeyend->second)
                                        {
                                            m_map.erase(behindend);                                                                     // Prune redundant successor node
                                        }
                                    }
                                }
                            }
                            break;
                        }
                    };

                    if(!endforend)
                    {
                        m_map.emplace_hint(nextIt, keyEnd, m_valBegin);                                                                 // Ensure default end boundary is set
                    }
                }

                if(m_map.size() == 1)
                {
                    m_map.clear();                                                                                                      // Prevent invalid single-node state
                }
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                m_map.merge(temp);                                                                                                      // Roll back map state on construction/allocation failure
            }
            catch(...)
            {
                std::cerr << "Undefined Error" << '\n';
                m_map.merge(temp);                                                                                                      // Roll back map state on construction/allocation failure
            }
        };

        /*!
            @brief Performs interval assignment by navigating downward and processing boundaries starting from lower_bound.

            @tparam V_forward Forwarding type for the value, constrained to match value type V.
            @param keyBegin Start key of the interval (inclusive).
            @param keyEnd End key of the interval (exclusive).
            @param val The value to assign across the interval.

            @details **Betrachtung:** 
                        `assignv3` setzt auf eine zielgerichtete Startposition mittels `lower_bound`, 
                        um von dort aus sequenziell die linke Seite, den relevanten Bereich und die rechte 
                        Intervallgrenze anzupassen oder zu bereinigen.

            @details **Regeln:** 
                        - Nutzt einen initialen `lower_bound(keyBegin)`, um den relevanten Bereich im Baum direkt zu finden.
                        - Behandelt Sonderfälle separat (z. B. wenn das Intervall komplett außerhalb des aktuellen Map-Endes 
                        oder ganz am linken Rand liegt).
                        - Führt über eine `while`-Schleife ein iteratives Löschen (`m_map.erase`) von überschriebenen 
                        Zwischenknoten durch und setzt das rechte Ende (`keyEnd`) inklusive des End-Wertes (`m_valBegin` oder Erbschaft).
                        - Ein abschließender Bereinigungsschritt (*"Besenstrich"*) entfernt redundante Endknoten oder leert 
                        die Map bei einem reinen Default-Zustand.

            @details **Erkenntnisse:** 
                        Diese Variante bietet eine sehr direkte und kompakte Handhabung der Intervall-Logik, 
                        wobei durch die strukturierte Behandlung der Ränder die Map-Integrität zu jedem Zeitpunkt gewahrt bleibt.
        */
        template<typename V_forward>
        void assignv3 (K const &keyBegin, K const &keyEnd, V_forward &&val)
        {
            auto itBegin = m_map.lower_bound(keyBegin);
            
            typename std::map<K,V>::iterator DeleteStart = m_map.begin();

            //--------------------------------------------------------------Right boundary / Map end behavior
            if(itBegin == m_map.end())
            {
                auto itKeyBegin = m_map.emplace_hint(m_map.end(), keyBegin, std::forward<V_forward>(val));
                auto itKeyEnd = m_map.emplace_hint(itKeyBegin, keyEnd, m_valBegin);
                return;
            }
            else
            {
                const bool singleinterval = (keyEnd < itBegin->first);                                          // Check if this is a single internal interval insertion

                auto tempbegin = std::pair<K,V>(keyBegin, std::forward<V_forward>(val));                        // Initialize tempbegin with the new interval start key and value
                auto tempend = std::pair<K,V>(keyEnd, m_valBegin);                                              // Initialize tempend with default fallback value; correct value is determined dynamically

                //----------------------------------------------------------Left boundary handling
                if(itBegin == m_map.begin())
                {
                    if(singleinterval)                                                                          // Single interval entirely to the left of the first element
                    {
                        auto keyendit = m_map.insert(m_map.begin(), tempend);
                        m_map.insert(keyendit, tempbegin);

                        return;
                    }
                    if(keyBegin < itBegin->first)                                                               // KeyBegin lies strictly before the first system key
                    {
                        m_map.insert(m_map.begin(), tempbegin);
                    }  
                    else                                                                                        // KeyBegin lands exactly on the first system key
                    {
                        itBegin->second = val;
                        DeleteStart = std::next(itBegin);
                    }
                }
                else                                                                                            // Handling insertion within existing map segments
                {
                    auto leftofbegin = std::prev(itBegin);
                    if(singleinterval)                                                                          // Handle splitting/encapsulating a single existing segment
                    {
                        tempend.second = leftofbegin->second;

                        if(!(leftofbegin->second == val))
                        {
                            auto keyendit = m_map.insert(m_map.begin(), tempend);
                            m_map.insert(keyendit, tempbegin);
                        }
                        return;
                    }

                    DeleteStart = itBegin;                                                                      // Default starting point for range deletion

                    if(!(leftofbegin->second == val))                                                           // Check if the predecessor value differs
                    {   

                        if(keyBegin < itBegin->first)                                                           // Insertion falls between predecessor and current bound
                        {
                            m_map.insert(itBegin, tempbegin);
                        }
                        else                                                                                    // Insertion lands directly on current bound
                        {
                            itBegin->second = val;
                            DeleteStart = std::next(itBegin);

                            if(DeleteStart != m_map.end())
                            {
                                if(keyEnd < DeleteStart->first)
                                {
                                    return;
                                }
                            }
                        }
                    }
                }
            };

            bool EndInserted = false;
            //-------------------------------------------------------------Right boundary search and range cleanup
            while(DeleteStart != m_map.end())
            {
                auto next_key = std::next(DeleteStart);                                                         // Cache successor iterator

                if(next_key != m_map.end())
                {
                    if(keyEnd < next_key->first)                                                                // Check if end boundary falls before the next element
                    {
                        EndInserted = true;
                        if(!(DeleteStart->second == val))
                        {
                            m_map.insert(next_key, std::pair<K,V>(keyEnd, DeleteStart->second));
                        }
                        if(!(!(keyEnd < DeleteStart->first) && !(DeleteStart->first < keyEnd)) || ((!(keyEnd < DeleteStart->first) && !(DeleteStart->first < keyEnd)) && DeleteStart->second == val))
                            m_map.erase(DeleteStart);                  
                        break;
                    }
                }

                m_map.erase(DeleteStart);
                DeleteStart = next_key;                                                                         // Advance to the next element in the deletion range
            }

            if(!EndInserted && DeleteStart == m_map.end() && m_map.size() >= 1)                                 // Ensure map tail terminates with m_valBegin if cleared up to the end
            {
                m_map.insert(m_map.end(), std::pair<K,V>(keyEnd, m_valBegin));
            };
            
            if (m_map.size() >= 2)                                                                              // Final canonicalization pass: remove redundant elements at map tail
            {
                auto lastIt = std::prev(m_map.end());
                auto prevIt = std::prev(lastIt);

                if (prevIt->second == lastIt->second)
                {
                    m_map.erase(lastIt);
                }
            }

            if(m_map.size() == 1) m_map.clear();                                                                // Clear map if only the default fallback state remains
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
            @brief Performs interval assignment moving upward through the map using forward iteration.

            @tparam V_forward Forwarding type for the value, constrained to match value type V.
            @param keyBegin Start key of the interval (inclusive).
            @param keyEnd End key of the interval (exclusive).
            @param val The value to assign across the interval.

            @details **Betrachtung:** 
                        `assignv4` nähert sich der Intervallzuweisung von der linken Seite der Map an und 
                        durchläuft die Segmente aufwärts, um Start- und Endgrenzen präzise zu setzen sowie 
                        überlappende Bereiche zu bereinigen.

            @details **Regeln:** 
                        - Vorab-Prüfung des rechten Randes (`rbegin()`), um bei Neuzuweisungen hinter dem letzten 
                        bekannten Key direkt einzufügen.
                        - Iterativer Vorwärtsdurchlauf ab `begin()`, wobei dynamisch zwischen `keyBegin` und `keyEnd` 
                        umgeschaltet wird (`FokusKey`).
                        - Gezieltes Ersetzen oder Löschen von überschriebenen Zwischenknoten basierend auf dem Zustand 
                        (`END`) und Wertvergleichen.
                        - Ein abschließender Bereinigungsschritt (*"Besenstrich"*) eliminiert redundante Endknoten.

            @details **Erkenntnisse:** 
                        Diese Vorwärts-Iterationsstrategie bietet eine alternative, rein sequenzielle Aufwärts-Logik, 
                        welche die Map-Struktur in einem Durchlauf konsistent hält.
        */
        template<typename V_forward>
        void assignv4 (K const &keyBegin, K const &keyEnd, V_forward &&val)
        {
            {   // Pre-check and handling for right boundary (beyond last element)
                auto Searchsection = m_map.rbegin();

                if(Searchsection != m_map.rend())
                {
                    if(Searchsection->first < keyBegin)
                    {
                        m_map.emplace_hint(m_map.end(), keyBegin, std::forward<V_forward>(val));
                        m_map.emplace_hint(m_map.end(), keyEnd, m_valBegin);
                        return;
                    }
                }
                else
                {
                    m_map.emplace_hint(m_map.end(), keyBegin, std::forward<V_forward>(val));
                    m_map.emplace_hint(m_map.end(), keyEnd, m_valBegin);

                    return;
                }
            }
            {   // Main forward traversal pass
                auto Searchsection = m_map.begin();
                auto Searchprev = m_map.begin();

                auto FokusKey = keyBegin;
                bool END = false;
                bool EndHandled = false;            

                while (Searchsection != m_map.end())
                {
                    bool Skip = false;
                    bool erased = false;
                    
                    auto next_section = std::next(Searchsection);

                    if(next_section != m_map.end())
                    {
                        if(FokusKey < next_section->first)                                                                  // Check if successor key is greater than the current focus key
                        {
                            if(EqualKey(FokusKey, Searchsection->first))                                                    // Landing directly on an existing key boundary
                            {
                                if(!END)
                                {
                                    if(!(Searchprev->second == val))
                                    {
                                        Searchsection->second = val;                                                        // Overwrite value at start boundary if it differs
                                    }
                                    
                                    FokusKey = keyEnd;
                                    END = true;
                                }
                                else
                                {
                                    if(Searchsection->second == val) { m_map.erase(Searchsection); }                        // If landing on keyEnd with an identical value, remove redundant node and exit
                                    
                                    return;
                                }
                            }
                            else                                                                                            // Landing inside an existing interval segment
                            {
                                bool SeachCurrentEqual = Searchsection->second == val;

                                bool Search_Smaller_Ley = FokusKey < Searchsection->first;
                                if(!END)
                                {
                                    if(Search_Smaller_Ley || !(SeachCurrentEqual))
                                    {
                                        m_map.emplace_hint(Searchsection, FokusKey, std::forward<V_forward>(val));
                                    }
                                    if(Search_Smaller_Ley || ((SeachCurrentEqual) && (keyEnd < next_section->first)))
                                    {
                                        Skip = true;
                                    }
                                    FokusKey = keyEnd;                                                                      // Switch focus to interval end key
                                    END = true;                                
                                }
                                else
                                {
                                    if(Search_Smaller_Ley && !(Searchprev->second == val))
                                    {
                                        m_map.emplace_hint(Searchsection, FokusKey, Searchprev->second);
                                        if(Searchprev->second == Searchsection->second)
                                        {
                                            m_map.erase(Searchprev);
                                        }
                                    }
                                    else if((SeachCurrentEqual) && (m_map.begin() != Searchsection))
                                    {
                                        if(std::prev(Searchsection)->second == val)
                                            m_map.erase(Searchsection);
                                    }    
                                    else if(!(SeachCurrentEqual) && (!(Searchprev->second == val) || (Searchsection->first < FokusKey)))
                                    {
                                        m_map.emplace_hint(next_section, FokusKey, Searchsection->second);
                                        m_map.erase(Searchsection);
                                    }                                                          
                                    
                                    return;
                                }
                            }                        
                        }
                        else
                        {
                            if(END)                                                                                         // Deletion is permitted only after passing keyBegin (END is active)
                            {
                                m_map.erase(Searchsection);
                                erased = true;
                            }
                        }                     
                    }
                    else
                    {                    
                        if(FokusKey < Searchsection->first)
                        {
                            m_map.emplace_hint(Searchsection, FokusKey, Searchprev->second);
                            return;
                        }
                        if(END)
                        {
                            m_map.erase(Searchsection);
                        }
                    }
                    if(!Skip)
                    {
                        if(erased) Searchprev = next_section;
                        else Searchprev = Searchsection;
                        Searchsection = next_section;       
                    }  
                }
                if(!EndHandled)
                {
                    m_map.emplace_hint(Searchsection, FokusKey, m_valBegin);
                }

                if (m_map.size() >= 2)                                                                                      // Final canonicalization pass: remove redundant elements at map tail
                {
                    auto lastIt = std::prev(m_map.end());
                    auto prevIt = std::prev(lastIt);

                    if (prevIt->second == lastIt->second)
                    {
                        m_map.erase(lastIt);
                    }
                }

                if(m_map.size() == 1) m_map.clear();
            }
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
                reutrn (--it)->second;
            }
        };
    };
};

