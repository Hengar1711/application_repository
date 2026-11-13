#pragma once
/*!
    @file ImGuiElement.hpp
    @author your name (you@domain.com)
    @brief 
    @version 0.1
    @date 2026-11-13

    @copyright Copyright (c) 2026

*/
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include <map>
#include <chrono>
#include <iostream>
#include <sstream>

#include "intervall_map.hpp"
#include "intervall_map_sps_ref.hpp"

//-------------------------------------------Support Classes
bool counting = true;

class KeySeperation
{
public:
    int m_val; // Absolute Immutability für den Map-Schlüssel!

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
    explicit KeySeperation(int val) : m_val(val)
    {
        if(counting) Key_ctor_count++;
    };

    // 2. Copy-Konstruktor
    KeySeperation(const KeySeperation& other) : m_val(other.m_val)
    {
        if(counting) Key_copy_ctor_count++;
    };

    // 3. Move-Konstruktor
    KeySeperation(KeySeperation&& other) noexcept(false) : m_val(other.m_val) 
    {
        if(counting) Key_move_ctor_count++;
    };

    // 4. Copy-Assignment (Achtung: const m_val kann nicht neu zugewiesen werden, 
    // aber in dieser Testklasse simulieren wir die Zuweisung über Neuanlage oder Logik, 
    // falls ein Zuweisungsoperator überhaupt aufgerufen werden darf).
    KeySeperation& operator=(const KeySeperation& other) 
    {
        if (this != &other) 
        {
            if(counting) Key_copy_assign_count++;
            // Da m_val const ist, geht hier nur Placement-New oder wir lassen Copy-Assign weg,
            // wenn der Key komplett unveränderlich sein soll. Für den Test lassen wir es defensiv:
            m_val = other.m_val;
        }
        return *this;
    };

    // 5. Move-Assignment
    KeySeperation& operator=(KeySeperation&& other) noexcept(false) 
    {
        if (this != &other) 
        {
            if(counting) Key_move_assign_count++;
            m_val = other.m_val;
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
        return m_val < input.m_val;
    };

    // Gleichheitsoperator strikt gelöscht
    bool operator==(const KeySeperation& input) = delete;

    operator int*()
    {
        return &m_val;
    };
};

struct ValueSeperation 
{
    char m_val[2];

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

    explicit ValueSeperation(char val) : m_val(val)
    {
        if(counting) Value_ctor_count++;
    };

    ValueSeperation(const ValueSeperation& other) : m_val(other.m_val[0])
    {
        if(counting) Value_copy_ctor_count++;
    };

    ValueSeperation(ValueSeperation&& other) noexcept(false) : m_val(other.m_val[0]) 
    {
        if(counting) Value_move_ctor_count++;
    };

    ValueSeperation& operator=(const ValueSeperation& other) 
    {
        if(counting) Value_copy_assign_count++;

        m_val[0] = other.m_val[0];
        return *this;
    };

    ValueSeperation& operator=(ValueSeperation&& other) noexcept(false) 
    {
        if(counting) Value_move_assign_count++;

        m_val[0] = other.m_val[0];
        return *this;
    };

    virtual ~ValueSeperation() 
    {
        if(counting) Value_dtor_count++;
    };

    bool operator==(const ValueSeperation& input) const 
    {
        if(counting) Value_compare_count++;
        return m_val[0] == input.m_val[0];
    };

    bool operator!=(const ValueSeperation& input) const  = delete;
    bool operator<(const ValueSeperation& input) const  = delete;
    bool operator>(const ValueSeperation& input) const  = delete;

    operator size_t()
    {
        return IM_ARRAYSIZE(m_val);
    };

    operator char*()
    {
        return m_val;
    };
};

std::map<std::size_t, int64_t> Diagnostig_Time[4];

PC::interval_map<KeySeperation, ValueSeperation> MapContainer ('h');
SPS::interval_map<KeySeperation, ValueSeperation> SPSContainer (MapContainer.m_map, MapContainer.m_valBegin);

long long min_time = std::numeric_limits<long long>::max();
long long max_time = 0;
long long total_time = 0;
long long run_count = 0;
int ausreizerphase = 0;

// Tracking für jede der 9 Phasen (0 bis 8)
long long total_time_per_state[9] = {0};
long long count_per_state[9] = {0};
long long max_time_per_state[9] = {0};

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

//-------------------------------------------Gui Functions
void Window_Mappresentation()
{
    ImGui::Begin("Mapview");

    std::stringstream ss;

    // 1. Basiswert anzeigen
    ss << "Default (m_valBegin): '" << MapContainer.m_valBegin.m_val << "'";
    ImGui::TextUnformatted(ss.str().c_str());
    ss.str(""); ss.clear(); // Stream für die nächste Zeile zurücksetzen

    ImGui::Separator();

    // 2. Logische Intervalle aufbereiten
    ImGui::TextDisabled("Logische Intervalle:");
    if (MapContainer.m_map.empty()) 
    {
        ss << "  [-inf .. +inf)  |  '" << MapContainer.m_valBegin.m_val << "'";
        ImGui::TextUnformatted(ss.str().c_str());
        ss.str(""); ss.clear();
    } 
    else 
    {
        auto it = MapContainer.m_map.begin();
        ss << "  [-inf .. " << it->first.m_val << ")  |  '" << MapContainer.m_valBegin.m_val << "'";
        ImGui::TextUnformatted(ss.str().c_str());
        ss.str(""); ss.clear();

        while (it != MapContainer.m_map.end()) 
        {
            auto next_it = std::next(it);
            if (next_it != MapContainer.m_map.end()) 
            {
                ss << "  [" << it->first.m_val << " .. " << next_it->first.m_val << ")  |  '" << it->second.m_val << "'";
            } 
            else 
            {
                ss << "  [" << it->first.m_val << " .. +inf)  |  '" << it->second.m_val << "'";
            }
            ImGui::TextUnformatted(ss.str().c_str());
            ss.str(""); ss.clear();
            ++it;
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // 3. Rohe std::map-Knoten anzeigen
    ss << "Raw std::map Nodes (" << MapContainer.m_map.size() << "):";
    ImGui::TextUnformatted(ss.str().c_str());
    ss.str(""); ss.clear();

    for (const auto& [k, v] : MapContainer.m_map) 
    {
        ss << "  Key: " << k.m_val << "  |  Val: '" << v.m_val << "'";
        ImGui::TextUnformatted(ss.str().c_str());
        ss.str(""); ss.clear();
    }

    ImGui::End();
};

void Window_Inputhandling()
{
    ImGui::Begin("Inputcontroll");

        // Setting Up the Variables for Intervall Input
        static KeySeperation Keybegin = KeySeperation(0);
        static KeySeperation Keyend = KeySeperation(0);
        static ValueSeperation singleChar = ValueSeperation('h');

        // Handle the Input Value for the Intervall Information
        ImGui::InputInt("Keybegin", Keybegin);
        ImGui::InputInt("Keyend", Keyend);
        ImGui::InputText("Char Content", singleChar, singleChar);

        // Generating the Selection Level of the Diffrent Assigns
        ImGui::Text("The Diffrent Assigns: ");
        if(ImGui::Button("Vs 1"))
        {
            MapContainer.assignselection = 1;
            auto MemSize = MapContainer.m_map.size();

            auto startcount = std::chrono::steady_clock::now();        

            MapContainer.assign(Keybegin, Keyend, singleChar);

            auto endcount = std::chrono::steady_clock::now();

            if(Diagnostig_Time[0][MemSize] == 0)
            {
                Diagnostig_Time[0][MemSize] = std::chrono::duration_cast<std::chrono::nanoseconds>(endcount - startcount).count();
            }
            else
            {
                Diagnostig_Time[0][MemSize] = (Diagnostig_Time[0][MemSize] + std::chrono::duration_cast<std::chrono::nanoseconds>(endcount - startcount).count()) / 2;
            }
        }
        ImGui::SameLine();
        if(ImGui::Button("Vs 2"))
        {
            MapContainer.assignselection = 2;
            auto MemSize = MapContainer.m_map.size();

            auto startcount = std::chrono::steady_clock::now();        

            MapContainer.assign(Keybegin, Keyend, singleChar);

            auto endcount = std::chrono::steady_clock::now();

            if(Diagnostig_Time[1][MemSize] == 0)
            {
                Diagnostig_Time[1][MemSize] = std::chrono::duration_cast<std::chrono::nanoseconds>(endcount - startcount).count();
            }
            else
            {
                Diagnostig_Time[1][MemSize] = (Diagnostig_Time[1][MemSize] + std::chrono::duration_cast<std::chrono::nanoseconds>(endcount - startcount).count()) / 2;
            }
        }
        ImGui::SameLine();
        if(ImGui::Button("Vs 3"))
        {
            MapContainer.assignselection = 3;
            auto MemSize = MapContainer.m_map.size();

            auto startcount = std::chrono::steady_clock::now();        

            MapContainer.assign(Keybegin, Keyend, singleChar);

            auto endcount = std::chrono::steady_clock::now();

            if(Diagnostig_Time[2][MemSize] == 0)
            {
                Diagnostig_Time[2][MemSize] = std::chrono::duration_cast<std::chrono::nanoseconds>(endcount - startcount).count();
            }
            else
            {
                Diagnostig_Time[2][MemSize] = (Diagnostig_Time[2][MemSize] + std::chrono::duration_cast<std::chrono::nanoseconds>(endcount - startcount).count()) / 2;
            }
        }
        ImGui::SameLine();
        if(ImGui::Button("Vs 4"))
        {
            MapContainer.assignselection = 4;
            auto MemSize = MapContainer.m_map.size();

            auto startcount = std::chrono::steady_clock::now();        

            MapContainer.assign(Keybegin, Keyend, singleChar);

            auto endcount = std::chrono::steady_clock::now();

            if(Diagnostig_Time[3][MemSize] == 0)
            {
                Diagnostig_Time[3][MemSize] = std::chrono::duration_cast<std::chrono::nanoseconds>(endcount - startcount).count();
            }
            else
            {
                Diagnostig_Time[3][MemSize] = (Diagnostig_Time[3][MemSize] + std::chrono::duration_cast<std::chrono::nanoseconds>(endcount - startcount).count()) / 2;
            }
        }
        if(ImGui::Button("Vs SPS"))
        {
            SPSContainer.assign(Keybegin, Keyend, singleChar);
        }

    ImGui::End();
};

void Window_Debugging_Infos()
{
    ImGui::Begin("Diagnostic");

    std::stringstream Buffer;

    // Diagnostic of the PC System Setup
    ImGui::SeparatorText("Speed-Analyse (Map-Size / Avg-Time)");
    int j = 0;
    for(auto Diag : Diagnostig_Time)
    {
        j += 1;
        Buffer.str(""); Buffer << "Version " << j << std::endl; 
        ImGui::SeparatorText(Buffer.str().c_str());
        for(auto [size, data] : Diag)
        {
            Buffer.str("");
            Buffer << "    " << size << "     | " << data << " nanoseconds" << std::endl;
            ImGui::Text(Buffer.str().c_str());
        }
    }

    // Diagnostic of the SPS System Setup
    ImGui::SeparatorText("Phase-Analyse (Avb / Max)");
    for(int i = 0; i < 9; ++i) 
    {
        if(count_per_state[i] > 0) 
        {
            long long avg_phase_time = total_time_per_state[i] / count_per_state[i];
            Buffer.str("");
            Buffer << "State " << i << " (" << Convert_state(i) << "): " << "Calls = " << count_per_state[i] 
                      << " | Avg = " << avg_phase_time << " ns" << " | Max = " << max_time_per_state[i] << " ns\n";
            ImGui::Text(Buffer.str().c_str());
        }
    }
    long long avg_time = (run_count > 0) ? (total_time / run_count) : 0;
    Buffer.str("");
    Buffer << "Runs: " << run_count << " | Min: " << min_time << " ns" << " | Max: " << max_time << " ns" << " | Avg: " << avg_time << " ns" << std::endl;
    Buffer << "Der Ausreizer war " << Convert_state(ausreizerphase) << std::endl;
    ImGui::Text(Buffer.str().c_str());

    // Diagnostic of the Compares, Constructions, Assigns and Destruction
    ImGui::SeparatorText("Lifestyle & Stats");
    ImGui::SeparatorText("Key Stats");
    Buffer.str("");Buffer << "Compare (<) : " << KeySeperation::Key_compare_count << std::endl;
    ImGui::Text(Buffer.str().c_str());

    Buffer.str(""); Buffer << "Constructor : " << KeySeperation::Key_ctor_count << std::endl;
    ImGui::Text(Buffer.str().c_str());

    Buffer.str(""); Buffer << "Copy Ctor : " << KeySeperation::Key_copy_ctor_count << std::endl;
    ImGui::Text(Buffer.str().c_str());

    Buffer.str(""); Buffer << "Copy Assign : " << KeySeperation::Key_copy_assign_count << std::endl;
    ImGui::Text(Buffer.str().c_str());

    Buffer.str(""); Buffer << "Move Ctor : " << KeySeperation::Key_move_ctor_count << std::endl;
    ImGui::Text(Buffer.str().c_str());

    Buffer.str(""); Buffer << "Move Assign : " << KeySeperation::Key_move_assign_count << std::endl;
    ImGui::Text(Buffer.str().c_str());

    Buffer.str(""); Buffer << "Destructors : " << KeySeperation::Key_dtor_count << std::endl;
    ImGui::Text(Buffer.str().c_str());

    ImGui::SeparatorText("Value Stats");
    Buffer << "Compare (==) : " << ValueSeperation::Value_compare_count << std::endl;
    ImGui::Text(Buffer.str().c_str());

    Buffer.str(""); Buffer << "Constructor : " << ValueSeperation::Value_ctor_count << std::endl;
    ImGui::Text(Buffer.str().c_str());

    Buffer.str(""); Buffer << "Copy Ctor : " << ValueSeperation::Value_copy_ctor_count << std::endl;
    ImGui::Text(Buffer.str().c_str());

    Buffer.str(""); Buffer << "Copy Assign : " << ValueSeperation::Value_copy_assign_count << std::endl;
    ImGui::Text(Buffer.str().c_str());

    Buffer.str(""); Buffer << "Move Ctor : " << ValueSeperation::Value_move_ctor_count << std::endl;
    ImGui::Text(Buffer.str().c_str());

    Buffer.str(""); Buffer << "Move Assign : " << ValueSeperation::Value_move_assign_count << std::endl;
    ImGui::Text(Buffer.str().c_str());

    Buffer.str(""); Buffer << "Destructors : " << ValueSeperation::Value_dtor_count << std::endl;
    ImGui::Text(Buffer.str().c_str());

    ImGui::End();
};


void Debugger_Interface()
{
    Window_Mappresentation();
    Window_Inputhandling();
    Window_Debugging_Infos();
    if(SPSContainer.filled_queue())
    {
        counting = true;
        auto start = std::chrono::steady_clock::now();

        SPSContainer.run();

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            
        counting = true;

        int active_state = SPSContainer.workingstate;
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
    }    
};