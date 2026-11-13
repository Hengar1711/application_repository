/*!
    @file Test_Version_Zyklus.cpp
    @author your name (you@domain.com)
    @brief 
    @version 0.1
    @date 2026-11-13

    @copyright Copyright (c) 2026
*/
#include "../../include/intervall_map_sps.hpp"

using namespace SPS;

enum class CrashPhase 
{
    None = 0,
    Constructor = 1,
    CopyCtor = 2,
    MoveCtor = 3,
    CopyAssign = 4,
    MoveAssign = 5,
    Compare = 6
};

bool counting = false;

class KeySeperation
{
public:
    const int m_val; // Absolute Immutability für den Map-Schlüssel!
    CrashPhase m_crash_phase;


    // Statische Diagnose-Zähler
    inline static int Key_ctor_count = 0;
    inline static int Key_copy_ctor_count = 0;
    inline static int Key_move_ctor_count = 0;
    inline static int Key_copy_assign_count = 0;
    inline static int Key_move_assign_count = 0;
    inline static int Key_compare_count = 0;
    inline static int Key_dtor_count = 0;

    static void reset_diagnostics() 
    {
        Key_ctor_count = 0;
        Key_copy_ctor_count = 0;
        Key_move_ctor_count = 0;
        Key_copy_assign_count = 0;
        Key_move_assign_count = 0;
        Key_compare_count = 0;
        Key_dtor_count = 0;
    };

    // 1. Normaler Konstruktor (Kein Default-Konstruktor!)
    explicit KeySeperation(int val, CrashPhase phase = CrashPhase::None) : m_val(val), m_crash_phase(phase) 
    {
        if(counting) Key_ctor_count++;
        if (m_crash_phase == CrashPhase::Constructor) 
        {
            throw 5; 
        }
    };

    // 2. Copy-Konstruktor
    KeySeperation(const KeySeperation& other) : m_val(other.m_val), m_crash_phase(other.m_crash_phase) 
    {
        if(counting) Key_copy_ctor_count++;
        if (m_crash_phase == CrashPhase::CopyCtor) 
        {
            throw 5;
        }
    };

    // 3. Move-Konstruktor
    KeySeperation(KeySeperation&& other) noexcept(false) : m_val(other.m_val), m_crash_phase(other.m_crash_phase) 
    {
        if(counting) Key_move_ctor_count++;
        if (m_crash_phase == CrashPhase::MoveCtor) 
        {
            throw 5;
        }
    };

    // 4. Copy-Assignment (Achtung: const m_val kann nicht neu zugewiesen werden, 
    // aber in dieser Testklasse simulieren wir die Zuweisung über Neuanlage oder Logik, 
    // falls ein Zuweisungsoperator überhaupt aufgerufen werden darf).
    KeySeperation& operator=(const KeySeperation& other) 
    {
        if (this != &other) 
        {
            if(counting) Key_copy_assign_count++;
            if (m_crash_phase == CrashPhase::CopyAssign || other.m_crash_phase == CrashPhase::CopyAssign) 
            {
                throw 5;
            }
            // Da m_val const ist, geht hier nur Placement-New oder wir lassen Copy-Assign weg,
            // wenn der Key komplett unveränderlich sein soll. Für den Test lassen wir es defensiv:
            const_cast<int&>(m_val) = other.m_val;
            m_crash_phase = other.m_crash_phase;
        }
        return *this;
    };

    // 5. Move-Assignment
    KeySeperation& operator=(KeySeperation&& other) noexcept(false) 
    {
        if (this != &other) 
        {
            if(counting) Key_move_assign_count++;
            if (m_crash_phase == CrashPhase::MoveAssign || other.m_crash_phase == CrashPhase::MoveAssign) 
            {
                throw 5;
            }
            const_cast<int&>(m_val) = other.m_val;
            m_crash_phase = other.m_crash_phase;
        }
        return *this;
    };

    // Destruktor
    virtual ~KeySeperation() 
    {
        if(counting) Key_dtor_count++;
    };

    // Strict Weak Ordering Operator <
    bool operator<(const KeySeperation& input) const 
    {
        if(counting) Key_compare_count++;
        if (m_crash_phase == CrashPhase::Compare || input.m_crash_phase == CrashPhase::Compare) 
        {
            throw 5;
        }
        return m_val < input.m_val;
    };

    // Gleichheitsoperator strikt gelöscht
    bool operator==(const KeySeperation& input) = delete;
};

struct ValueSeperation 
{
    char m_val;
    CrashPhase m_crash_phase;

    inline static int Value_ctor_count = 0;
    inline static int Value_copy_ctor_count = 0;
    inline static int Value_move_ctor_count = 0;
    inline static int Value_copy_assign_count = 0;
    inline static int Value_move_assign_count = 0;
    inline static int Value_compare_count = 0;
    inline static int Value_dtor_count = 0;

    static void reset_diagnostics() 
    {
        Value_ctor_count = 0;
        Value_copy_ctor_count = 0;
        Value_move_ctor_count = 0;
        Value_copy_assign_count = 0;
        Value_move_assign_count = 0;
        Value_compare_count = 0;
        Value_dtor_count = 0;
    };

    explicit ValueSeperation(char val, CrashPhase phase = CrashPhase::None) : m_val(val), m_crash_phase(phase) 
    {
        if(counting) Value_ctor_count++;
        if (m_crash_phase == CrashPhase::Constructor) 
        {
            throw 5; 
        }
    };

    ValueSeperation(const ValueSeperation& other) : m_val(other.m_val), m_crash_phase(other.m_crash_phase) 
    {
        if(counting) Value_copy_ctor_count++;
        if (m_crash_phase == CrashPhase::CopyCtor) 
        {
            throw 5;
        }
    };

    ValueSeperation(ValueSeperation&& other) noexcept(false) : m_val(other.m_val), m_crash_phase(other.m_crash_phase) 
    {
        if(counting) Value_move_ctor_count++;
        if (m_crash_phase == CrashPhase::MoveCtor) 
        {
            throw 5;
        }
    };

    ValueSeperation& operator=(const ValueSeperation& other) 
    {
        if(counting) Value_copy_assign_count++;
        if (m_crash_phase == CrashPhase::CopyCtor) // oder eigene CrashPhase::CopyAssign
        {
            throw 5;
        }
        m_val = other.m_val;
        m_crash_phase = other.m_crash_phase;
        return *this;
    };

    ValueSeperation& operator=(ValueSeperation&& other) noexcept(false) 
    {
        if(counting) Value_move_assign_count++;
        if (m_crash_phase == CrashPhase::MoveCtor) 
        {
            throw 5;
        }
        m_val = other.m_val;
        m_crash_phase = other.m_crash_phase;
        return *this;
    };

    virtual ~ValueSeperation() 
    {
        if(counting) Value_dtor_count++;
    };

    bool operator==(const ValueSeperation& input) const 
    {
        if(counting) Value_compare_count++;
        return m_val == input.m_val;
    };

    bool operator!=(const ValueSeperation& input) const  = delete;
    bool operator<(const ValueSeperation& input) const  = delete;
    bool operator>(const ValueSeperation& input) const  = delete;    
};


interval_map<KeySeperation, ValueSeperation> Testmap('h');

class Erwartungen_Binding
{public:
    ValueSeperation Content;
    KeySeperation keystart;

    Erwartungen_Binding(ValueSeperation Content, KeySeperation keystart = KeySeperation{std::numeric_limits<int>::max()}) : Content(Content), keystart(keystart) {};

    operator ValueSeperation() const
    {
        return Content;
    };

    operator KeySeperation()
    {
        return keystart;
    };

    bool has_keystart()
    {
        return keystart.m_val != std::numeric_limits<int>::max();
    };
};

struct Testinfo
{
    KeySeperation keybegin;
    KeySeperation keyend;
    ValueSeperation input;
    std::vector<Erwartungen_Binding> erwartung;

    Testinfo(KeySeperation keybegin, KeySeperation keyend, ValueSeperation input, std::vector<Erwartungen_Binding> erwartung) : 
    keybegin(keybegin), keyend(keyend), input(input), erwartung(erwartung)
    {};
};

typedef KeySeperation KEY;
typedef ValueSeperation VALUE;
std::vector<Testinfo> Testcase1 =
{
    {KEY{-20},  KEY{20}, VALUE{'l'}, {{VALUE{'l'}, KEY{-20}}, {VALUE{'h'},  KEY{ 20}}}},
    {KEY{-38},  KEY{10}, VALUE{'a'}, {{VALUE{'a'}, KEY{-38}}, {VALUE{'l'},  KEY{ 10}}, {VALUE{'h'},  KEY{ 20}}}},
    {KEY{-38},  KEY{32}, VALUE{'a'}, {{VALUE{'a'}, KEY{-38}}, {VALUE{'h'},  KEY{ 32}}}},
    {KEY{  5},  KEY{21}, VALUE{'a'}, {{VALUE{'a'}, KEY{-38}}, {VALUE{'h'},  KEY{ 32}}}},
    {KEY{-23},  KEY{ 8}, VALUE{'a'}, {{VALUE{'a'}, KEY{-38}}, {VALUE{'h'},  KEY{ 32}}}},
    {KEY{ -2},  KEY{33}, VALUE{'c'}, {{VALUE{'a'}, KEY{-38}}, {VALUE{'c'},  KEY{ -2}}, {VALUE{'h'},  KEY{ 33}}}},
    {KEY{-33},  KEY{39}, VALUE{'m'}, {{VALUE{'a'}, KEY{-38}}, {VALUE{'m'},  KEY{-33}}, {VALUE{'h'},  KEY{ 39}}}},
    {KEY{  5},  KEY{32}, VALUE{'c'}, {{VALUE{'a'}, KEY{-38}}, {VALUE{'m'},  KEY{-33}}, {VALUE{'c'},  KEY{  5}}, {VALUE{'m'},  KEY{32}}, {VALUE{'h'},  KEY{39}}}},
    {KEY{-17},  KEY{-2}, VALUE{'l'}, {{VALUE{'a'}, KEY{-38}}, {VALUE{'m'},  KEY{-33}}, {VALUE{'l'},  KEY{-17}}, {VALUE{'m'},  KEY{-2}}, {VALUE{'c'},  KEY{ 5}}, {VALUE{'m'},  KEY{32}}, {VALUE{'h'},  KEY{39}}}},
    // ============================================================================
    // PHASE 1: COMPARISON PHASE (operator<)
    // ============================================================================
    // The test verifies that the tree traversal via operator< finds the correct insertion boundary without mutating the map state. Expected: Normal execution, matching the exact expected map state.
    {KEY{-5, CrashPhase::Compare},  KEY{20}, VALUE{'p'}, {{VALUE{'a'}, KEY{-38}}, {VALUE{'m'},  KEY{-33}}, {VALUE{'l'},  KEY{-17}}, {VALUE{'m'},  KEY{-2}}, {VALUE{'c'},  KEY{ 5}}, {VALUE{'m'},  KEY{32}}, {VALUE{'h'},  KEY{39}}}},

    // ============================================================================
    {KEY{-25}, KEY{ 25}, VALUE{'b'}, {{VALUE{'a'}, KEY{-38}}, {VALUE{'m'}, KEY{-33}}, {VALUE{'b'}, KEY{-25}}, {VALUE{'c'}, KEY{ 25}}, {VALUE{'m'}, KEY{ 32}}, {VALUE{'h'}, KEY{ 39}}}},
    {KEY{  0}, KEY{ 36}, VALUE{'c'}, {{VALUE{'a'}, KEY{-38}}, {VALUE{'m'}, KEY{-33}}, {VALUE{'b'}, KEY{-25}}, {VALUE{'c'}, KEY{  0}}, {VALUE{'m'}, KEY{ 36}}, {VALUE{'h'}, KEY{ 39}}}},
    {KEY{ 21}, KEY{ 23}, VALUE{'b'}, {{VALUE{'a'}, KEY{-38}}, {VALUE{'m'}, KEY{-33}}, {VALUE{'b'}, KEY{-25}}, {VALUE{'c'}, KEY{  0}}, {VALUE{'b'}, KEY{ 21}}, {VALUE{'c'}, KEY{ 23}}, {VALUE{'m'}, KEY{36}}, {VALUE{'h'}, KEY{39}}}},
    {KEY{-21}, KEY{ 26}, VALUE{'x'}, {{VALUE{'a'}, KEY{-38}}, {VALUE{'m'}, KEY{-33}}, {VALUE{'b'}, KEY{-25}}, {VALUE{'x'}, KEY{-21}}, {VALUE{'c'}, KEY{ 26}}, {VALUE{'m'}, KEY{ 36}}, {VALUE{'h'}, KEY{39}}}},
    {KEY{-17}, KEY{ 27}, VALUE{'a'}, {{VALUE{'a'}, KEY{-38}}, {VALUE{'m'}, KEY{-33}}, {VALUE{'b'}, KEY{-25}}, {VALUE{'x'}, KEY{-21}}, {VALUE{'a'}, KEY{-17}}, {VALUE{'c'}, KEY{ 27}}, {VALUE{'m'}, KEY{36}}, {VALUE{'h'}, KEY{39}}}},
    {KEY{ -1}, KEY{  5}, VALUE{'b'}, {{VALUE{'a'}, KEY{-38}}, {VALUE{'m'}, KEY{-33}}, {VALUE{'b'}, KEY{-25}}, {VALUE{'x'}, KEY{-21}}, {VALUE{'a'}, KEY{-17}}, {VALUE{'b'}, KEY{ -1}}, {VALUE{'a'}, KEY{ 5}}, {VALUE{'c'}, KEY{27}}, {VALUE{'m'}, KEY{36}}, {VALUE{'h'}, KEY{39}}}},
    {KEY{-24}, KEY{  6}, VALUE{'a'}, {{VALUE{'a'}, KEY{-38}}, {VALUE{'m'}, KEY{-33}}, {VALUE{'b'}, KEY{-25}}, {VALUE{'a'}, KEY{-24}}, {VALUE{'c'}, KEY{ 27}}, {VALUE{'m'}, KEY{ 36}}, {VALUE{'h'}, KEY{39}}}},
    {KEY{-24}, KEY{  7}, VALUE{'a'}, {{VALUE{'a'}, KEY{-38}}, {VALUE{'m'}, KEY{-33}}, {VALUE{'b'}, KEY{-25}}, {VALUE{'a'}, KEY{-24}}, {VALUE{'c'}, KEY{ 27}}, {VALUE{'m'}, KEY{ 36}}, {VALUE{'h'}, KEY{39}}}},
    {KEY{-31}, KEY{-26}, VALUE{'a'}, {{VALUE{'a'}, KEY{-38}}, {VALUE{'m'}, KEY{-33}}, {VALUE{'a'}, KEY{-31}}, {VALUE{'m'}, KEY{-26}}, {VALUE{'b'}, KEY{-25}}, {VALUE{'a'}, KEY{-24}}, {VALUE{'c'}, KEY{27}}, {VALUE{'m'}, KEY{36}}, {VALUE{'h'}, KEY{39}}}},
    {KEY{-35}, KEY{ 12}, VALUE{'b'}, {{VALUE{'a'}, KEY{-38}}, {VALUE{'b'}, KEY{-35}}, {VALUE{'a'}, KEY{ 12}}, {VALUE{'c'}, KEY{ 27}}, {VALUE{'m'}, KEY{ 36}}, {VALUE{'h'}, KEY{ 39}}}},
            
};

std::map<std::size_t, int64_t> Diagnostig_Time;

/*!
    @brief Validates the actual state of the interval map against expected test bindings.
    
    @param ref Reference to the underlying std::map of the interval_map.
    @param input Vector of expected bindings (`Erwartungen_Binding`) representing the map state.
    @param keybegin Start key of the tested interval.
    @param keyend End key of the tested interval.
    @param kontent Input content value assigned during the test step.

    @details **Betrachtung:**
                `InterpredSupport` fungiert als Assertions- und Validierungshelfer. Sie vergleicht 
                die Anzahl der Segmente sowie deren Werte und Schlüssel exakt mit den Vorgaben des Testfalls.

    @details **Regeln:**
                - Wirft spezifische Integer-Fehlercodes (`2`, `3`, `4`), wenn Unstimmigkeiten bei der 
                  Größe, den Werten oder den Schlüsseln auftreten[cite: 1], um den Fehler im Testlauf 
                  klar zu lokalisieren.
*/
void InterpredSupport(std::map<KeySeperation, ValueSeperation>& ref, std::vector<Erwartungen_Binding> input, KeySeperation keybegin, KeySeperation keyend, ValueSeperation kontent)
{
    if(ref.size() != input.size())
    {
        std::cout << "Das Interval von " << keybegin.m_val << " bis " << keyend.m_val << " mit dem Inhalt " << kontent.m_val << " könnte nicht mal stimmen." << std::endl;
    
        throw int(2);
    }
    int iteration = 0;
    for(auto [key, value] : ref)
    {
        if(!(value == input[iteration].Content))
        {
            std::cout << "Das Interval von " << keybegin.m_val << " bis " << keyend.m_val << " mit dem Inhalt " << kontent.m_val << " hat bei der Erwartung " << input[iteration].Content.m_val << " stattdessen " << value.m_val << std::endl;
            throw int(3);
        }
        if(input[iteration].has_keystart())
        {
            if(key.m_val != input[iteration].keystart.m_val)
            {
                std::cout << "Das Interval von " << keybegin.m_val << " bis " << keyend.m_val << " mit dem Inhalt " << kontent.m_val << " hat bei der Erwartung " << input[iteration].keystart.m_val << " stattdessen " << key.m_val << std::endl;
                throw int(4);
            }
        }
        iteration++;
    }
};

int Profiling = 0;

void SPS::IntervalMapTest()
{
    InterpredSupport(Testmap.m_map, Testcase1[Profiling].erwartung, Testcase1[Profiling].keybegin, Testcase1[Profiling].keyend, Testcase1[Profiling].input);
};

inline std::string Convert_state(int i)
{
    switch (i)
    {
        case 0:  return "IDLE / Finished";
        case 1:  return "INIT";
        case 2:  return "SEARCH";
        case 3:  return "Cano";
        case 4:  return "SEARCH_END_CHECK";
        case 5:  return "EMPLACE_BEGIN";
        case 6:  return "EMPLACE_END";
        case 7:  return "ERASE";
        case 8:  return "SET NEXT";
        default: return "UNKNOWN_STATE";
    }
};


/*!
        @brief Benchmark and profiling runner for deterministic interval map validation.

        @details **Betrachtung:** 
                    Um die Echtzeittauglichkeit und Jitter-Freiheit zu verifizieren, misst dieser 
                    Testlauf die Ausführungszeit jedes einzelnen State-Machine-Zyklus in Nanosekunden.
                    Zudem wird gezielt erfasst, welcher Zustand (`ausreizerphase`) den zeitlichen 
                    Spitzenwert (`max_time`) verursacht hat.

        @details **Regeln:** 
                    - Erfasst Min-, Max- und Durchschnittszeiten über alle Zyklen.
                    - Identifiziert den schwersten Zustand als "Ausreizer", um Worst-Case-Szenarien 
                      (WCET) für die SPS-Umgebung zu analysieren.
    */
int main(int argc, char**argv)
{
    // Ungenutzte Argumente explizit kennzeichnen (optional)
    (void)argc;
    (void)argv;


    long long min_time = std::numeric_limits<long long>::max();
    long long max_time = 0;
    long long total_time = 0;
    long long run_count = 0;
    int ausreizerphase = 0;

    // Tracking für jede der 9 Phasen (0 bis 8)
    long long total_time_per_state[9] = {0};
    long long count_per_state[9] = {0};
    long long max_time_per_state[9] = {0};

    // Aufgeräumt: const reference vermeidet unnötige Kopien
    for(auto Case : Testcase1)
    {
        Testmap.assign(Case.keybegin, Case.keyend, Case.input);
    }

    try
    {        
        while(Testmap.filled_queue())
        { 
            int active_state = Testmap.workingstate; // Zustand merken, bevor run() ihn evtl. ändert
            
            counting = true;
            auto start = std::chrono::steady_clock::now();
            
            Testmap.run();

            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            counting = false;

            // Phasen-spezifische Metriken aktualisieren
            total_time_per_state[active_state] += duration;
            count_per_state[active_state]++;
            if (duration > max_time_per_state[active_state]) 
            {
                max_time_per_state[active_state] = duration;
            }

            // Gesamtmessungen...
            if (duration < min_time) min_time = duration;
            if (duration > max_time) 
            {
                max_time = duration; 
                ausreizerphase = active_state; 
            }
            total_time += duration;
            run_count++;
            
            if(Testmap.workingstate == 0)
            {
                IntervalMapTest();
                Profiling++;
                if(Profiling == 9) Profiling++;
            }            
        }
    }
    catch(int errorCode)
    {
        std::cout << " FAIL: Test abgebrochen mit Error-Code: " << errorCode << std::endl;

        return errorCode;
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << '\n';
        std::cout << "FAIL: Unerwartete Standard-Exception: " << e.what() << std::endl;
        return 99;
    }
    catch(...)
    {
        std::cout << "FAIL: Unbekannte Exception!" << std::endl;
        return 255;
    }
    
    

    long long avg_time = (run_count > 0) ? (total_time / run_count) : 0;
    std::cout << "\n================ PHASEN-ANALYSE (AVG / MAX) ================\n";
    for(int i = 0; i < 9; ++i) 
    {
        if(count_per_state[i] > 0) 
        {
            long long avg_phase_time = total_time_per_state[i] / count_per_state[i];
            std::cout << "State " << i << " (" << Convert_state(i) << "): "
                      << "Calls = " << count_per_state[i] 
                      << " | Avg = " << avg_phase_time << " ns"
                      << " | Max = " << max_time_per_state[i] << " ns\n";
        }
    }
    std::cout << "Runs: " << run_count << " | Min: " << min_time << " ns" << " | Max: " << max_time << " ns" << " | Avg: " << avg_time << " ns" << std::endl;
    std::cout << "Der Ausreizer war " << Convert_state(ausreizerphase) << std::endl;

    std::cout << "========================================" << std::endl;

    std::cout << "\n================ LIFESTYLE & STATS ================\n";
    std::cout << "[Key-Stats]\n"
            << "  Vergleiche (<) : " << KeySeperation::Key_compare_count << "\n"
            << "  Konstrukteure  : " << KeySeperation::Key_ctor_count << "\n"
            << "  Copy Ctor      : " << KeySeperation::Key_copy_ctor_count << "\n"
            << "  Copy Assign    : " << KeySeperation::Key_copy_assign_count << "\n"
            << "  Move Ctor      : " << KeySeperation::Key_move_ctor_count << "\n"
            << "  Move Assign    : " << KeySeperation::Key_move_assign_count << "\n"
            << "  Destrukteure   : " << KeySeperation::Key_dtor_count << "\n";

    std::cout << "---------------------------------------------------\n";

    std::cout << "[Value-Stats]\n"
            << "  Vergleiche (==) : " << ValueSeperation::Value_compare_count << "\n"
            << "  Konstrukteure   : " << ValueSeperation::Value_ctor_count << "\n"
            << "  Copy Ctor       : " << ValueSeperation::Value_copy_ctor_count << "\n"
            << "  Copy Assign     : " << ValueSeperation::Value_copy_assign_count << "\n"
            << "  Move Ctor       : " << ValueSeperation::Value_move_ctor_count << "\n"
            << "  Move Assign     : " << ValueSeperation::Value_move_assign_count << "\n"
            << "  Destrukteure    : " << ValueSeperation::Value_dtor_count << "\n";
    std::cout << "===================================================\n\n";
    return 0;
};