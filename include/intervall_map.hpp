#pragma once

#include <map>
#include <chrono>

template <typename K, typename V>
class interval_map
{
    friend void IntervalMapTest();
    V m_valBegin;
    std::map<K,V> m_map;
    bool Debugging = false;

public:

    //Consturctor associates whole range of K with val
    template<typename V_forward>
    interval_map(V_forward &&val) : m_valBegin(std::forward<V_forward>(val)) {};

    /*!
        @brief  Assign value val to interfal [keyBegin, keyEnd]
                Overwrite previos values in this intervall
                Confomring to the Cüü Standard Libarary convention the interval
                includes keyBegin, but excludes keyEnd
                if !(keyBegin < keyEnd), this designates an empty interval and assign must do nothing

        @details    Zustände 
                    (A)Value ist Default | (a)Value ist nicht Default
                    (B)Key End Liegt hinter dem Letzten Eintrag |(b) Key End liegt vor dem letzten Eintrag
                    (C)key Beginn liegt hinter dem ersten Eintrag |(c) Key Beginn liegt vor dem ersten Eintrag

                    "Einschränkungen" meide Dopplungen
                    Bedeutet Beginn != Default
                    Bedeutet KeyEnd == Default

                    Kombinationen Situationen
                    (1)ABC (2)AbC (3)ABc (4)Abc
                    (5)aBC (6)abC (7)aBc (8)abc

                    1 Key Begin Einfügen -> Löschung ab Key Begin Next bis End 
                    2 Erstllen KeyEnd mit prev Value -> Löschen von begin bis keyEnd -> Einfügen von KeyBegin
                    3 Löschen der Gesamten Map
                    4 Erstellen KeyEnd mit prev Value -> löschen von begin bis KeyEnd
                    5 Füge KeyBegin -> Lösche von KeyBegin und End -> Füge KeyBegin und KeyEnd ein
                    6 Füge KeyEnd mit prev value ein -> Füge KeyBegin ein -> Lösche von KeyBegin next bis KeyEnd
                    7 Lösche die Map -> Füge KeyBegin ein -> Füge Keyend ein
                    8 Füge KeyEnd mit prev value ein -> Füge KeyBegin ein -> Lösche von KeyBegin next bis KeyEnd
    */
    template<typename V_forawrd>
    void assign (K const &keyBegin, K const &keyEnd, V_forawrd &&val) requires (std::is_same<std::remove_cvref_t<V_forawrd>, V>::value)
    {
    };

    /*!
        @brief  First version focusing strictly on boundary handling.
                Built as an ad-hoc prototype that guarantees exception safety
                before modifying the main map..

        @details Hier lege ich den Fokus auf die Grenzen
                 Das heißt es ist eine bewusste Verletzung gegen die logN Aufbau.
                 Zur Erfüllung von Strong Exception nutze ich das Pair vorher zu erstellen
                 Danach inserte ich es
                 Nach dem Insert - wird mithilfe von std::prev und std::next sowie erase range gearbeitet
        
        @tparam V_forawrd 
        @param keyBegin 
        @param keyEnd 
        @param val 
    */
    template<typename V_forawrd>
    void assignv1 (K const &keyBegin, K const &keyEnd, V_forawrd &&val)
    {
        //--------------------------------------------------------------Setup temporary variables
        std::pair<K,V> tempbegin;                                                                                                   // Hold start key-value pair
        std::pair<K,V> tempend;                                                                                                     // Hold end key-value pair

        //--------------------------------------------------------------Create pairs safely (Strong Exception Guarantee)
        try
        {
            tempbegin = std::pair<K,V>(keyBegin, std::forward<V_forward>(val));                                                     // Try creating start pair; aborts if K or V copy fails
            tempend = std::pair<K,V>(keyEnd, m_valBegin);                                                                           // Try creating end pair with default value
        }
        catch(const std::exception& e)                                                                                              // Catch object construction errors
        {
            if(Debugging) std::cerr << e.what() << '\n';                                                                            // Optional Print debug error
            return;                                                                                                                 // Do nothing on failure (map stays untouched)
        }
        
        //--------------------------------------------------------------Insert start boundary
        auto [tempbeginIT, insertedstart] = m_map.insert(tempbegin);                                                                // Insert start key and store iterator + insert status

        if(std::next(tempbeginIT) != m_map.end())                                                                                   // Check if there is a next element
        {
            if(keyEnd < std::next(tempbeginIT)->first)                                                                              // Check if interval falls inside a single existing segment
            {
                if(!insertedstart && tempbeginIT->second == val)                                                                    // Check if key exists and value is already identical
                {
                    return;                                                                                                         // Exit early since no change is needed
                }
                tempend.second = std::prev(tempbeginIT)->second;                                                                    // Use previous interval value for end key
            }
        }

        auto [tempendIT, insertedend] = m_map.insert(tempend);                                                                      // Insert end key to set right boundary
        
        //--------------------------------------------------------------Set up range to erase
        auto DeleteStart = tempbeginIT;                                                                                             // Mark start of erase range
        auto DeleteEnd = tempendIT;                                                                                                 // Mark end of erase range

        //--------------------------------------------------------------Fix left boundary
        if(!insertedstart && !(tempbeginIT->second == tempbegin.second))                                                            // Check if start key existed with a different value
        {
            tempbeginIT->second = tempbegin.second;                                                                                 // Update value at existing key
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
                if(!(tempbegin.second == std::prev(tempendIT)->second))                                                             // Check if value changes before end key
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
                // Prüfe ob ende und nachfolger identisch sind 
                if(tempendIT->second == tempbehindendIT->second || (!insertedend && DeleteEnd->second == DeleteStart->second) && (DeleteStart->second == val))  // Check for duplicate values at the end
                {
                    DeleteEnd = tempbehindendIT;                                                                                                                // Include duplicate end key in erase range
                }
            }
        }    

        //--------------------------------------------------------------Erase duplicate range and clean up
        if(DeleteStart != DeleteEnd) m_map.erase(DeleteStart, DeleteEnd);                                                                                       // Erase elements in range
        
        // Finaler Besenstrich am Ende der Map 
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
        @brief Gehe mithilfe von lower_bound die Schleife aufwärts durch
        
        @tparam V_forawrd 
        @param keyBegin 
        @param keyEnd 
        @param val 
    */
    template<typename V_forward>
    void assignv2 (K const &keyBegin, K const &keyEnd, V_forward &&val)
    {

    };

    /*!
        @brief Gehe mithilfe von lower_bound die Schleife abwärts durch
        
        @tparam V_forawrd 
        @param keyBegin 
        @param keyEnd 
        @param val 
    */
    template<typename V_forward>
    void assignv3 (K const &keyBegin, K const &keyEnd, V_forward &&val)
    {

    };

    /*!
        @brief Gehe mithilfe von upper_bound die Schleife Aufwärts durch
        
        @tparam V_forawrd 
        @param keyBegin 
        @param keyEnd 
        @param val 
    */
    template<typename V_forward>
    void assignv4 (K const &keyBegin, K const &keyEnd, V_forward &&val)
    {

    };

    /*!
        @brief Gehe mithilfe von upper_bound die Schleife abwärts durch
        
        @tparam V_forawrd 
        @param keyBegin 
        @param keyEnd 
        @param val 
    */
    template<typename V_forward>
    void assignv5 (K const &keyBegin, K const &keyEnd, V_forward &&val)
    {

    };

    /*!
        @brief Gehe mithilfe von eigener logn Schleife aufwärts durch
        
        @tparam V_forawrd 
        @param keyBegin 
        @param keyEnd 
        @param val 
    */
    template<typename V_forward>
    void assignv6 (K const &keyBegin, K const &keyEnd, V_forward &&val)
    {

    };

    /*!
        @brief Gehe mithilfe von eigener logn Schleife abwärts durch
        
        @tparam V_forawrd 
        @param keyBegin 
        @param keyEnd 
        @param val 
    */
    template<typename V_forward>
    void assignv7 (K const &keyBegin, K const &keyEnd, V_forward &&val)
    {

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