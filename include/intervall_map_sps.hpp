#pragma once

#include <map>
#include <chrono>

template <typename K, typename V>
class interval_map
{
    friend void IntervalMapTest();
    V m_valBegin;
    std::map<K,V> m_map;

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

    void run ()
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