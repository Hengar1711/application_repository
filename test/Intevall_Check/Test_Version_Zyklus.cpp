/*!
    @file Test_Version_Zyklus.cpp
    @author your name (you@domain.com)
    @brief Test and benchmarking suite for the deterministic state machine-based interval_map.
    @version 0.1
    @date 2026-11-12

    @details **Betrachtung:**
                Diese Testdatei steuert den automatisierten Validierungs- und Benchmark-Zyklus 
                der `interval_map`. Sie füttert die Map mit einer umfangreichen Testvektor-Matrix 
                (`Testcase1`), führt die Zustandsmaschine schrittweise aus und misst dabei präzise 
                die Ausführungszeiten im Nanosekundenbereich.

    @details **Regeln:**
                - Validiert nach jeder abgeschlossenen Operation (Zustand 0 / IDLE) die Integrität 
                  der Map gegen die hinterlegten Erwartungswerte (`Erwartungen_Binding`).
                - Erfasst umfassende Performance-Metriken (Minimum, Maximum, Durchschnitt sowie 
                  den worst-case Ausreizer der jeweiligen Phasen).

    @copyright Copyright (c) 2026
*/
#include "../../include/intervall_map_sps.hpp"

using namespace SPS;

//-----------------------------------------------------Variables
interval_map<int,char> Testmap('h');
std::map<std::size_t, int64_t> Diagnostig_Time;
int Profiling = 0;

//-----------------------------------------------------Supportclass
/*!
    @class Erwartungen_Binding
    @brief Holds expected interval mapping values and key boundaries for test validation.

    @details **Betrachtung:**
                Diese Hilfsklasse kapselt die Erwartungswerte für einzelne Testschritte. 
                Sie speichert den erwarteten Inhalt (`Content`) sowie den optionalen 
                Schlüsselstartpunkt (`keystart`), ab dem der Wert gültig ist.

    @details **Regeln:**
                - Standardmäßig ist `keystart` auf den maximalen Integer-Wert gesetzt, 
                  was bedeutet, dass kein expliziter Key-Start erzwungen wird.
                - Bietet implizite Konvertierungsoperatoren für den direkten Vergleich mit 
                  den Datentypen `char` und `int`.
*/
class Erwartungen_Binding
{
public:
    char Content;   ///< Expected value/content of the interval segment
    int keystart;   ///< Expected start key of the interval segment

    /*!
        @brief Constructor initializing content and an optional key start boundary.
        @param Content Expected content character.
        @param keystart Expected start key (defaults to numeric_limits<int>::max()).
    */
    Erwartungen_Binding(char Content, int keystart = std::numeric_limits<int>::max()) 
        : Content(Content), keystart(keystart) {};

    /*!
        @brief Implicit conversion operator to retrieve the expected content character.
    */
    operator char() const
    {
        return Content;
    };

    /*!
        @brief Implicit conversion operator to retrieve the expected key start.
    */
    operator int()
    {
        return keystart;
    };

    /*!
        @brief Checks whether a specific key start boundary has been defined.
        @return True if keystart is set (not numeric_limits<int>::max()), false otherwise.
    */
    bool has_keystart()
    {
        return keystart != std::numeric_limits<int>::max();
    };
};

/*!
    @struct Testinfo
    @brief Encapsulates input parameters and expected output states for a single test case.

    @details **Betrachtung:**
                Diese Datenstruktur repräsentiert einen einzelnen Testdurchlauf in der Testvektor-Matrix. 
                Sie definiert das zuzuweisende Intervall (`keybegin` bis `keyend`), den zu übergebenden 
                Wert (`input`) sowie die vollständige Liste der erwarteten Map-Segmente (`erwartung`).

    @details **Regeln:**
                - Dient als Basis für die Testfälle in `Testcase1`.
                - Die Erwartungsliste wird nach jeder Operation gegen den tatsächlichen Zustand 
                  der `interval_map` validiert.
*/
struct Testinfo
{
    int keybegin;                           ///< Start key of the test assignment interval (inclusive)
    int keyend;                             ///< End key of the test assignment interval (exclusive)
    char input;                             ///< Input character value assigned to the interval
    std::vector<Erwartungen_Binding> erwartung; ///< Expected map state represented as a vector of bindings
};

std::vector<Testinfo> Testcase1 =
{
    {-20,  20, 'l', {{'l', -20}, {'h',  20}}},
    {-38,  10, 'a', {{'a', -38}, {'l',  10}, {'h',  20}}},
    { 38, -32, 'p', {{'a', -38}, {'h',  32}}},
    {-38,  32, 'a', {{'a', -38}, {'h',  32}}},
    {  5,  21, 'a', {{'a', -38}, {'h',  32}}},
    {-23,   8, 'a', {{'a', -38}, {'h',  32}}},
    { -2,  33, 'c', {{'a', -38}, {'c',  -2}, {'h',  33}}},
    {-33,  39, 'm', {{'a', -38}, {'m', -33}, {'h',  39}}},
    {  5,  32, 'c', {{'a', -38}, {'m', -33}, {'c',   5}, {'m',  32}, {'h',  39}}},
    {-17,  -2, 'l', {{'a', -38}, {'m', -33}, {'l', -17}, {'m',  -2}, {'c',   5}, {'m',  32}, {'h',  39}}},
    {-25,  25, 'b', {{'a', -38}, {'m', -33}, {'b', -25}, {'c',  25}, {'m',  32}, {'h',  39}}},
    {  0,  36, 'c', {{'a', -38}, {'m', -33}, {'b', -25}, {'c',   0}, {'m',  36}, {'h',  39}}},
    { 21,  23, 'b', {{'a', -38}, {'m', -33}, {'b', -25}, {'c',   0}, {'b',  21}, {'c',  23}, {'m',  36}, {'h',  39}}},
    {-21,  26, 'x', {{'a', -38}, {'m', -33}, {'b', -25}, {'x', -21}, {'c',  26}, {'m',  36}, {'h',  39}}},
    {-17,  27, 'a', {{'a', -38}, {'m', -33}, {'b', -25}, {'x', -21}, {'a', -17}, {'c',  27}, {'m',  36}, {'h',  39}}},
    { -1,   5, 'b', {{'a', -38}, {'m', -33}, {'b', -25}, {'x', -21}, {'a', -17}, {'b',  -1}, {'a',   5}, {'c',  27}, {'m', 36}, {'h', 39}}},
    {-24,   6, 'a', {{'a', -38}, {'m', -33}, {'b', -25}, {'a', -24}, {'c',  27}, {'m',  36}, {'h',  39}}},
    {-24,   7, 'a', {{'a', -38}, {'m', -33}, {'b', -25}, {'a', -24}, {'c',  27}, {'m',  36}, {'h',  39}}},
    {-31, -26, 'a', {{'a', -38}, {'m', -33}, {'a', -31}, {'m', -26}, {'b', -25}, {'a', -24}, {'c',  27}, {'m',  36}, {'h', 39}}},
    {-35,  12, 'b', {{'a', -38}, {'b', -35}, {'a',  12}, {'c',  27}, {'m',  36}, {'h',  39}}},
    { -1,  20, 'b', {{'a', -38}, {'b', -35}, {'a',  20}, {'c',  27}, {'m',  36}, {'h',  39}}},
    {-28,  -7, 'l', {{'a', -38}, {'b', -35}, {'l', -28}, {'b',  -7}, {'a',  20}, {'c',  27}, {'m',  36}, {'h',  39}}},
    {  7,  21, 'a', {{'a', -38}, {'b', -35}, {'l', -28}, {'b',  -7}, {'a',   7}, {'c',  27}, {'m',  36}, {'h',  39}}},
    {-29,  37, 'a', {{'a', -38}, {'b', -35}, {'a', -29}, {'m',  37}, {'h',  39}}},
    {-27,  25, 'x', {{'a', -38}, {'b', -35}, {'a', -29}, {'x', -27}, {'a',  25}, {'m',  37}, {'h',  39}}},
    {  1,  11, 'm', {{'a', -38}, {'b', -35}, {'a', -29}, {'x', -27}, {'m',   1}, {'x',  11}, {'a',  25}, {'m',  37}, {'h', 39}}},
    {-34, -28, 'm', {{'a', -38}, {'b', -35}, {'m', -34}, {'a', -28}, {'x', -27}, {'m',   1}, {'x',  11}, {'a',  25}, {'m', 37}, {'h', 39}}},
    { -3,  24, 'c', {{'a', -38}, {'b', -35}, {'m', -34}, {'a', -28}, {'x', -27}, {'c',  -3}, {'x',  24}, {'a',  25}, {'m', 37}, {'h', 39}}},
    { -3,  28, 'a', {{'a', -38}, {'b', -35}, {'m', -34}, {'a', -28}, {'x', -27}, {'a',  -3}, {'m',  37}, {'h',  39}}},
    {-17,  -1, 'c', {{'a', -38}, {'b', -35}, {'m', -34}, {'a', -28}, {'x', -27}, {'c', -17}, {'a',  -1}, {'m',  37}, {'h', 39}}},
    {-33,   3, 'x', {{'a', -38}, {'b', -35}, {'m', -34}, {'x', -33}, {'a',   3}, {'m',  37}, {'h',  39}}},
    {-17,   6, 'c', {{'a', -38}, {'b', -35}, {'m', -34}, {'x', -33}, {'c', -17}, {'a',   6}, {'m',  37}, {'h',  39}}},
    { 12,  33, 'b', {{'a', -38}, {'b', -35}, {'m', -34}, {'x', -33}, {'c', -17}, {'a',   6}, {'b',  12}, {'a',  33}, {'m',  37}, {'h', 39}}},
    { 17,  40, 'b', {{'a', -38}, {'b', -35}, {'m', -34}, {'x', -33}, {'c', -17}, {'a',   6}, {'b',  12}, {'h',  40}}},
    { 11,  19, 'l', {{'a', -38}, {'b', -35}, {'m', -34}, {'x', -33}, {'c', -17}, {'a',   6}, {'l',  11}, {'b',  19}, {'h', 40}}},
    {  0,  19, 'a', {{'a', -38}, {'b', -35}, {'m', -34}, {'x', -33}, {'c', -17}, {'a',   0}, {'b',  19}, {'h',  40}}},
    { -1,  32, 'c', {{'a', -38}, {'b', -35}, {'m', -34}, {'x', -33}, {'c', -17}, {'b',  32}, {'h',  40}}},
    {-32,  11, 'm', {{'a', -38}, {'b', -35}, {'m', -34}, {'x', -33}, {'m', -32}, {'c',  11}, {'b',  32}, {'h',  40}}},
    {-32, -19, 'm', {{'a', -38}, {'b', -35}, {'m', -34}, {'x', -33}, {'m', -32}, {'c',  11}, {'b',  32}, {'h',  40}}},
    {-19,   8, 'm', {{'a', -38}, {'b', -35}, {'m', -34}, {'x', -33}, {'m', -32}, {'c',  11}, {'b',  32}, {'h',  40}}},
    {-23, -17, 'l', {{'a', -38}, {'b', -35}, {'m', -34}, {'x', -33}, {'m', -32}, {'l', -23}, {'m', -17}, {'c',  11}, {'b', 32}, {'h', 40}}},
    {-10,   3, 'a', {{'a', -38}, {'b', -35}, {'m', -34}, {'x', -33}, {'m', -32}, {'l', -23}, {'m', -17}, {'a', -10}, {'m',  3}, {'c', 11}, {'b', 32}, {'h', 40}}},
    {  3,  36, 'x', {{'a', -38}, {'b', -35}, {'m', -34}, {'x', -33}, {'m', -32}, {'l', -23}, {'m', -17}, {'a', -10}, {'x',  3}, {'b', 36}, {'h', 40}}},
    {-17,  19, 'a', {{'a', -38}, {'b', -35}, {'m', -34}, {'x', -33}, {'m', -32}, {'l', -23}, {'a', -17}, {'x',  19}, {'b', 36}, {'h', 40}}},
    { 17,  37, 'c', {{'a', -38}, {'b', -35}, {'m', -34}, {'x', -33}, {'m', -32}, {'l', -23}, {'a', -17}, {'c',  17}, {'b', 37}, {'h', 40}}},
    {-18,  -4, 'm', {{'a', -38}, {'b', -35}, {'m', -34}, {'x', -33}, {'m', -32}, {'l', -23}, {'m', -18}, {'a',  -4}, {'c', 17}, {'b', 37}, {'h', 40}}},
    {-40,  -1, 'l', {{'l', -40}, {'a',  -1}, {'c',  17}, {'b',  37}, {'h',  40}}},
    { 29,  36, 'a', {{'l', -40}, {'a',  -1}, {'c',  17}, {'a',  29}, {'c',  36}, {'b',  37}, {'h',  40}}},
    {-13,   4, 'm', {{'l', -40}, {'m', -13}, {'a',   4}, {'c',  17}, {'a',  29}, {'c',  36}, {'b',  37}, {'h',  40}}},
    { -8,   5, 'a', {{'l', -40}, {'m', -13}, {'a',  -8}, {'c',  17}, {'a',  29}, {'c',  36}, {'b',  37}, {'h',  40}}},
    {-23,  -2, 'm', {{'l', -40}, {'m', -23}, {'a',  -2}, {'c',  17}, {'a',  29}, {'c',  36}, {'b',  37}, {'h',  40}}},
    { -9,  -5, 'x', {{'l', -40}, {'m', -23}, {'x',  -9}, {'m',  -5}, {'a',  -2}, {'c',  17}, {'a',  29}, {'c',  36}, {'b',  37}, {'h', 40}}},
    {-13, -11, 'c', {{'l', -40}, {'m', -23}, {'c', -13}, {'m', -11}, {'x',  -9}, {'m',  -5}, {'a',  -2}, {'c',  17}, {'a',  29}, {'c', 36}, {'b', 37}, {'h', 40}}},
    {-17,  10, 'x', {{'l', -40}, {'m', -23}, {'x', -17}, {'a',  10}, {'c',  17}, {'a',  29}, {'c',  36}, {'b',  37}, {'h',  40}}},
    { -7,   8, 'x', {{'l', -40}, {'m', -23}, {'x', -17}, {'a',  10}, {'c',  17}, {'a',  29}, {'c',  36}, {'b',  37}, {'h',  40}}},
    {-31,  29, 'm', {{'l', -40}, {'m', -31}, {'a',  29}, {'c',  36}, {'b',  37}, {'h',  40}}},
    {-38,  -4, 'a', {{'l', -40}, {'a', -38}, {'m',  -4}, {'a',  29}, {'c',  36}, {'b',  37}, {'h',  40}}},
    {-21,  14, 'm', {{'l', -40}, {'a', -38}, {'m', -21}, {'a',  29}, {'c',  36}, {'b',  37}, {'h',  40}}},
    {-31,  -6, 'a', {{'l', -40}, {'a', -38}, {'m',  -6}, {'a',  29}, {'c',  36}, {'b',  37}, {'h',  40}}},
    {  3,  28, 'x', {{'l', -40}, {'a', -38}, {'m',  -6}, {'x',   3}, {'m',  28}, {'a',  29}, {'c',  36}, {'b',  37}, {'h',  40}}},
    {-28,  29, 'a', {{'l', -40}, {'a', -38}, {'c',  36}, {'b',  37}, {'h',  40}}},
    {  4,  20, 'b', {{'l', -40}, {'a', -38}, {'b',   4}, {'a',  20}, {'c',  36}, {'b',  37}, {'h',  40}}},
    {-27,  21, 'm', {{'l', -40}, {'a', -38}, {'m', -27}, {'a',  21}, {'c',  36}, {'b',  37}, {'h',  40}}},
    { 31,  32, 'l', {{'l', -40}, {'a', -38}, {'m', -27}, {'a',  21}, {'l',  31}, {'a',  32}, {'c',  36}, {'b',  37}, {'h',  40}}},
    {-38,   3, 'a', {{'l', -40}, {'a', -38}, {'m',   3}, {'a',  21}, {'l',  31}, {'a',  32}, {'c',  36}, {'b',  37}, {'h',  40}}},
    {-34,  23, 'l', {{'l', -40}, {'a', -38}, {'l', -34}, {'a',  23}, {'l',  31}, {'a',  32}, {'c',  36}, {'b',  37}, {'h',  40}}},
    { -5,  29, 'm', {{'l', -40}, {'a', -38}, {'l', -34}, {'m',  -5}, {'a',  29}, {'l',  31}, {'a',  32}, {'c',  36}, {'b',  37}, {'h', 40}}},
    {-38,  -1, 'x', {{'l', -40}, {'x', -38}, {'m',  -1}, {'a',  29}, {'l',  31}, {'a',  32}, {'c',  36}, {'b',  37}, {'h',  40}}},
    {-19,  25, 'b', {{'l', -40}, {'x', -38}, {'b', -19}, {'m',  25}, {'a',  29}, {'l',  31}, {'a',  32}, {'c',  36}, {'b',  37}, {'h', 40}}},
    {-32, -25, 'l', {{'l', -40}, {'x', -38}, {'l', -32}, {'x', -25}, {'b', -19}, {'m',  25}, {'a',  29}, {'l',  31}, {'a',  32}, {'c', 36}, {'b', 37}, {'h', 40}}},
    {-30, -20, 'l', {{'l', -40}, {'x', -38}, {'l', -32}, {'x', -20}, {'b', -19}, {'m',  25}, {'a',  29}, {'l',  31}, {'a',  32}, {'c', 36}, {'b', 37}, {'h', 40}}},
    { 10,  20, 'x', {{'l', -40}, {'x', -38}, {'l', -32}, {'x', -20}, {'b', -19}, {'x',  10}, {'b',  20}, {'m',  25}, {'a',  29}, {'l', 31}, {'a', 32}, {'c', 36}, {'b', 37}, {'h', 40}}},
    { -9,  24, 'l', {{'l', -40}, {'x', -38}, {'l', -32}, {'x', -20}, {'b', -19}, {'l',  -9}, {'b',  24}, {'m',  25}, {'a',  29}, {'l', 31}, {'a', 32}, {'c', 36}, {'b', 37}, {'h', 40}}},
    {  7,  26, 'm', {{'l', -40}, {'x', -38}, {'l', -32}, {'x', -20}, {'b', -19}, {'l',  -9}, {'m',   7}, {'a',  29}, {'l',  31}, {'a', 32}, {'c', 36}, {'b', 37}, {'h', 40}}},
    {-21, -13, 'l', {{'l', -40}, {'x', -38}, {'l', -32}, {'b', -13}, {'l',  -9}, {'m',   7}, {'a',  29}, {'l',  31}, {'a',  32}, {'c', 36}, {'b', 37}, {'h', 40}}},
    {-17,  22, 'c', {{'l', -40}, {'x', -38}, {'l', -32}, {'c', -17}, {'m',  22}, {'a',  29}, {'l',  31}, {'a',  32}, {'c',  36}, {'b', 37}, {'h', 40}}},
    {-40, -37, 'b', {{'b', -40}, {'x', -37}, {'l', -32}, {'c', -17}, {'m',  22}, {'a',  29}, {'l',  31}, {'a',  32}, {'c',  36}, {'b', 37}, {'h', 40}}},
    {-16,   3, 'x', {{'b', -40}, {'x', -37}, {'l', -32}, {'c', -17}, {'x', -16}, {'c',   3}, {'m',  22}, {'a',  29}, {'l',  31}, {'a', 32}, {'c', 36}, {'b', 37}, {'h', 40}}},
    {  7,  34, 'x', {{'b', -40}, {'x', -37}, {'l', -32}, {'c', -17}, {'x', -16}, {'c',   3}, {'x',   7}, {'a',  34}, {'c',  36}, {'b', 37}, {'h', 40}}},
    {-40, -13, 'x', {{'x', -40}, {'c',   3}, {'x',   7}, {'a',  34}, {'c',  36}, {'b',  37}, {'h', 40}}},
    { 10,  25, 'x', {{'x', -40}, {'c',   3}, {'x',   7}, {'a',  34}, {'c',  36}, {'b',  37}, {'h', 40}}},
    { 20,  24, 'x', {{'x', -40}, {'c',   3}, {'x',   7}, {'a',  34}, {'c',  36}, {'b',  37}, {'h', 40}}},
    {-19,  -6, 'b', {{'x', -40}, {'b', -19}, {'x',  -6}, {'c',   3}, {'x',   7}, {'a',  34}, {'c',  36}, {'b',  37}, {'h',  40}}},
    {-12,  26, 'c', {{'x', -40}, {'b', -19}, {'c', -12}, {'x',  26}, {'a',  34}, {'c',  36}, {'b',  37}, {'h',  40}}},
    { -2,   9, 'l', {{'x', -40}, {'b', -19}, {'c', -12}, {'l',  -2}, {'c',   9}, {'x',  26}, {'a',  34}, {'c',  36}, {'b',  37}, {'h', 40}}},
    {-38, -24, 'a', {{'x', -40}, {'a', -38}, {'x', -24}, {'b', -19}, {'c', -12}, {'l',  -2}, {'c',   9}, {'x',  26}, {'a',  34}, {'c', 36}, {'b', 37}, {'h', 40}}},
    { 19,  40, 'm', {{'x', -40}, {'a', -38}, {'x', -24}, {'b', -19}, {'c', -12}, {'l',  -2}, {'c',   9}, {'m',  19}, {'h',  40}}},
    { -9,  -3, 'a', {{'x', -40}, {'a', -38}, {'x', -24}, {'b', -19}, {'c', -12}, {'a',  -9}, {'c',  -3}, {'l',  -2}, {'c',   9}, {'m', 19}, {'h', 40}}},
    { -9,  37, 'a', {{'x', -40}, {'a', -38}, {'x', -24}, {'b', -19}, {'c', -12}, {'a',  -9}, {'m',  37}, {'h',  40}}},
    {-36, -23, 'c', {{'x', -40}, {'a', -38}, {'c', -36}, {'x', -23}, {'b', -19}, {'c', -12}, {'a', - 9}, {'m',  37}, {'h',  40}}},
    {-16,  17, 'l', {{'x', -40}, {'a', -38}, {'c', -36}, {'x', -23}, {'b', -19}, {'l', -16}, {'a',  17}, {'m',  37}, {'h',  40}}},
    { 14,  34, 'x', {{'x', -40}, {'a', -38}, {'c', -36}, {'x', -23}, {'b', -19}, {'l', -16}, {'x',  14}, {'a',  34}, {'m',  37}, {'h', 40}}},
    { -4,   4, 'x', {{'x', -40}, {'a', -38}, {'c', -36}, {'x', -23}, {'b', -19}, {'l', -16}, {'x',  -4}, {'l',   4}, {'x',  14}, {'a', 34}, {'m', 37}, {'h', 40}}},
    {-30,  22, 'c', {{'x', -40}, {'a', -38}, {'c', -36}, {'x',  22}, {'a',  34}, {'m',  37}, {'h',  40}}},
    {-24,  33, 'x', {{'x', -40}, {'a', -38}, {'c', -36}, {'x', -24}, {'a',  34}, {'m',  37}, {'h',  40}}},
    { -6,  20, 'a', {{'x', -40}, {'a', -38}, {'c', -36}, {'x', -24}, {'a',  -6}, {'x',  20}, {'a',  34}, {'m',  37}, {'h',  40}}},
    {-11,  34, 'b', {{'x', -40}, {'a', -38}, {'c', -36}, {'x', -24}, {'b', -11}, {'a',  34}, {'m',  37}, {'h',  40}}},
    {-11,  13, 'b', {{'x', -40}, {'a', -38}, {'c', -36}, {'x', -24}, {'b', -11}, {'a',  34}, {'m',  37}, {'h',  40}}},
    {-11,  40, 'b', {{'x', -40}, {'a', -38}, {'c', -36}, {'x', -24}, {'b', -11}, {'h',  40}}},
    {-40, -28, 'x', {{'x', -40}, {'c', -28}, {'x', -24}, {'b', -11}, {'h',  40}}},
    { 18,  39, 'a', {{'x', -40}, {'c', -28}, {'x', -24}, {'b', -11}, {'a',  18}, {'b',  39}, {'h',  40}}},
    { 16,  19, 'm', {{'x', -40}, {'c', -28}, {'x', -24}, {'b', -11}, {'m',  16}, {'a',  19}, {'b',  39}, {'h',  40}}},
    {-40,  40, 'h', {}}
};


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
void InterpredSupport(std::map<int, char>& ref, std::vector<Erwartungen_Binding> input, int keybegin, int keyend, char kontent)
{
    if(ref.size() != input.size())
    {
        std::cout << "Das Interval von " << keybegin << " bis " << keyend << " mit dem Inhalt " << kontent << " könnte nicht mal stimmen." << std::endl;
    
        throw int(2);
    }
    int iteration = 0;
    for(auto [key, value] : ref)
    {
        if(value != input[iteration].Content)
        {
            std::cout << "Das Interval von " << keybegin << " bis " << keyend << " mit dem Inhalt " << kontent << " hat bei der Erwartung " << input[iteration].Content << " stattdessen " << value << std::endl;
            throw int(3);
        }
        if(input[iteration].has_keystart())
        {
            if(key != input[iteration].keystart)
            {
                std::cout << "Das Interval von " << keybegin << " bis " << keyend << " mit dem Inhalt " << kontent << " hat bei der Erwartung " << input[iteration].keystart << " stattdessen " << key << std::endl;
                throw int(4);
            }
        }
        iteration++;
    }
};

/*!
    @brief Executes the validation check for the current test profiling step.

    @details **Betrachtung:**
                Diese Funktion ruft `InterpredSupport` auf, um den aktuellen Zustand der Map 
                mit dem aktiven Testfall (`Testcase1[Profiling]`) abzugleichen.
*/
void SPS::IntervalMapTest()
{
    InterpredSupport(Testmap.m_map, Testcase1[Profiling].erwartung, Testcase1[Profiling].keybegin, Testcase1[Profiling].keyend, Testcase1[Profiling].input);  
};

/*!
    @brief Converts a state machine state integer into a human-readable description string.
    
    @param i Integer representing the current working state.
    @return std::string Readable name of the state.

    @details **Betrachtung:**
                Dient der Diagnose und Visualisierung, um den aktiven Zustand der State Machine 
                (z. B. `INIT`, `SEARCH`, `EMPLACE_BEGIN`, etc.) im Benchmark oder Debugger auszugeben.
*/
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
}

/*!
    @brief Main entry point for the interval_map state machine benchmark and test suite.

    @details **Betrachtung:**
                Der Einstiegspunkt initialisiert den Testdurchlauf, füttert die Map mit allen Testfällen 
                und führt die zustandsbasierte Ausführung (`Testmap.run()`) unter präziser Nanosekunden-Messung aus.

    @details **Regeln:**
                - Führt Laufzeit- und Performance-Analysen durch (Erfassung von Min-, Max- und Durchschnittszeiten).
                - Überwacht Ausreißer in spezifischen Phasen der State Machine.
                - Validiert nach Abschluss jedes Testschritts die Korrektheit über `IntervalMapTest()`.
*/

int main(int argc, char**argv)
{
    long long min_time = std::numeric_limits<long long>::max();
    long long max_time = 0;
    long long total_time = 0;
    long long run_count = 0;
    int ausreizerphase = 0;

    for(auto Case : Testcase1)
    {
        Testmap.assign(Case.keybegin, Case.keyend, Case.input);
    }

    try
    {
        while(Testmap.filled_queue())
        {
            auto start = std::chrono::steady_clock::now();
            
            Testmap.run();

            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

            // std::cout << "Comming state is " << Convert_state(Testmap.workingstate) << std::endl;

            if (duration < min_time) min_time = duration;
            if (duration > max_time) {max_time = duration; ausreizerphase = Testmap.workingstate; }
            total_time += duration;
            run_count++;
            
            if(Testmap.workingstate == 0)
            {
                IntervalMapTest();

                // std::cout << "Profiling Finished for  " << Profiling << std::endl;

                if(Profiling == 59) 
                    bool test = false;

                Profiling++;
                if(Profiling == 2) Profiling ++;
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
        std::cout << "Runs: " << run_count << " | Min: " << min_time << " ns" << " | Max: " << max_time << " ns" << " | Avg: " << avg_time << " ns" << std::endl;
        std::cout << "Der Ausreizer war " << Convert_state(ausreizerphase) << std::endl;
    return 0;
};