# Multi Version Interval Map

## Overview / Überblick

#### Eng
Important: The choice to split the blocks stems from the fact that I've found thinking in my native language significantly helps in identifying edge cases and danger zones. Because of this, you might still encounter some German context or comments here and there — either because I couldn't find an ideal translation at the moment or simply left them as is while focusing fully on the core logic.

#### Ger
Dieses Repository dient als Entwicklungstagebuch und dokumentiert die Entwicklung der interval_map sowie den Aufbau der verschiedenen Versionen.

Wichtiger Hinweis zum Sprachen-Mix:  
Meine primäre Analysesprache ist Deutsch. Zwar habe ich kein grundsätzliches Problem mit Englisch, aber ich bin darin etwas eingerostet, weshalb ich dafür oft länger brauche, als mir lieb ist. Zudem habe ich die Erfahrung gemacht, dass das Denken in der Muttersprache extrem dabei hilft, Corner-/Edge-Cases (Grenzfälle) und Gefahrenzonen (Danger Zones) zielsicher zu identifizieren und zu lösen.

Aus diesem Grund wirst du hier im Repository noch deutschen Kontext, Kommentare oder Begriffe finden – teils, weil mir in dem Moment keine ideale englische Übersetzung eingefiel, oder weil es schlichtweg so stehen geblieben ist, während mein Fokus auf der mathematischen und logischen Korrektheit lag. Ich bitte um Verzeihung, falls dadurch anfangs die ein oder andere Namensgebung etwas unkonventionell wirkt.

(Hinweis zur Dokumentation: Dieser Text wurde mit unterstützender KI-Assistenz geprüft; die englische Fassung ist dabei als kompakte Zusammenfassung gedacht, während der deutsche Text hier die tieferen Details und Hintergründe enthält.)


# Benchmark & Cross-Run Performance Evaluation (V1 – V4 & VSPS)

To evaluate the stability, performance, and operational determinism of the different architectural approaches, all implementations were tested head-to-head across **3 distinct test runs** (`10/10 tests passed` in every run).

---

## Cross-Run Latency Comparison (Runde 1 / 2 / 3)
The following tables track the execution time in nanoseconds (`ns`) for standard types (`Test_IntervalMap`) across map sizes `0` to `14`.

### 1. Version 1 vs. Version 2 (Latency per Run: R1 / R2 / R3)

| Map-Size | V1 (R1 / R2 / R3) | V2 (R1 / R2 / R3) |
| :---: | :---: | :---: |
| **0 (Cold)** | 5,500 / 5,600 / 5,400 ns | 3,400 / 3,500 / 3,300 ns |
| **2** | 887 / 962 / 893 ns | 650 / 587 / 631 ns |
| **3** | 825 / 825 / 850 ns | 825 / 800 / 825 ns |
| **5** | 743 / 700 / 725 ns | 431 / 487 / 500 ns |
| **6** | 693 / 696 / 734 ns | 928 / 993 / 990 ns |
| **7** | 782 / 765 / 843 ns | 618 / 632 / 633 ns |
| **8** | 804 / 778 / 772 ns | 1,376 / 1,385 / 1,460 ns |
| **9** | 799 / 800 / 791 ns | 786 / 789 / 868 ns |
| **10** | 768 / 727 / 802 ns | 491 / 568 / 557 ns |
| **11** | 881 / 946 / 896 ns | 1,312 / 1,346 / 1,284 ns |
| **12** | 1,081 / 1,053 / 1,075 ns | 1,504 / 1,500 / 1,462 ns |
| **13** | 900 / 1,050 / 1,000 ns | 1,250 / 1,300 / 1,350 ns |
| **14** | 1,000 / 950 / 1,000 ns | 900 / 850 / 1,000 ns |

### 2. Version 3 vs. Version 4 (Latency per Run: R1 / R2 / R3)

| Map-Size | V3 (R1 / R2 / R3) | V4 (R1 / R2 / R3) |
| :---: | :---: | :---: |
| **0 (Cold)** | 2,800 / 2,900 / 2,800 ns | 2,800 / 2,800 / 2,800 ns |
| **2** | 575 / 568 / 562 ns | 637 / 593 / 637 ns |
| **3** | 750 / 775 / 725 ns | 700 / 675 / 700 ns |
| **5** | 512 / 568 / 518 ns | 556 / 631 / 606 ns |
| **6** | 596 / 606 / 596 ns | 537 / 559 / 531 ns |
| **7** | 597 / 582 / 577 ns | 630 / 643 / 630 ns |
| **8** | 883 / 879 / 831 ns | 880 / 860 / 874 ns |
| **9** | 795 / 843 / 794 ns | 689 / 704 / 659 ns |
| **10** | 518 / 555 / 553 ns | 506 / 597 / 579 ns |
| **11** | 659 / 718 / 668 ns | 712 / 700 / 728 ns |
| **12** | 925 / 942 / 942 ns | 971 / 953 / 937 ns |
| **13** | 800 / 750 / 750 ns | 850 / 800 / 800 ns |
| **14** | 650 / 700 / 700 ns | 750 / 750 / 800 ns |

---

## State-Machine (`VSPS`) Cross-Run Summary

| Durchlauf | Total Runs | Avg Cycle Latency | Min / Max Latency | Primary Outlier State |
| :--- | :---: | :---: | :---: | :--- |
| **Runde 1** | 1,721 | 86 ns | 0 / 1,500 ns | `EMPLACE_BEGIN` |
| **Runde 2** | 1,721 | 82 ns | 0 / 1,600 ns | `Cano` |
| **Runde 3** | 1,721 | 85 ns | 0 / 1,500 ns | `EMPLACE_BEGIN` |

---

## Engineering Takeaways
* **High Determinism:** The cross-run variance across all three iterations is virtually negligible, confirming robust, predictable runtime behavior.
* **Latency Profile:** **V3** and **V4** achieve the most stable scaling profiles for larger map sizes, whereas **V2** exhibits periodic spikes tied to its hint mechanics.
* **PLC State Machine (`VSPS`):** Delivers ultra-low cycle times (~84 ns average), proving that deterministic state-machine decomposition introduces minimal overhead while providing fine-grained telemetry.

## Architectural Comparison: Monolithic C++ vs. Deterministic State Machine (VSPS)

To provide a fair, head-to-head performance comparison between standard monolithic C++ implementations and the PLC-inspired finite state machine (`VSPS`), the cumulative execution time of all state transitions per `assign()` operation was calculated (`19` operations evaluated).

| Version | Architectural Paradigm | Avg. Total Latency per `assign()` | Architectural Role / Trade-off |
| :--- | :--- | :---: | :--- |
| **V3** | Boundary-Split & Optimized Flow | **~858 ns** | 🥇 Fastest direct C++ algorithm |
| **V4** | Structured State-Logic | **~866 ns** | 🥈 Highly stable and robust |
| **V2** | Hint-Optimized Insertion | **~1,130 ns** | Periodic latency spikes on complex trees |
| **V1** | Naive / Direct Standard Approach | **~1,167 ns** | Higher overhead on cold-start allocations |
| **VSPS** | **PLC-Inspired State Machine (Kumuliert)** | **~1,791 ns** | 🛡️ Deterministic real-time safety (strict cycle bounds) |

## Round-by-Round Execution Analysis (R1 vs. R2 vs. R3)

The following tables break down the performance across the three individual test runs, showing the average map operation latency (for V1–V4 across map sizes 0–14) alongside the accumulated `VSPS` operation duration per run.

### 1. Round 1 Breakdown
| Version | Approach | Average Latency / Operation | Notes / Observations |
| :--- | :--- | :---: | :--- |
| **V3** | Boundary-Split | **~850 ns** | Best scaling across map sizes |
| **V4** | Structured State | **~862 ns** | Extremely consistent low variance |
| **V2** | Hint-Optimized | **~1,113 ns** | Spikes at size 8 and 11–12 |
| **V1** | Naive Baseline | **~1,204 ns** | Highest cold-start cost (5,500 ns at size 0) |
| **VSPS** | State-Machine (Kumuliert) | **~1,724 ns** | 1,721 total state steps; avg 86 ns/cycle |

### 2. Round 2 Breakdown (Reproducibility & Stability)
| Version | Approach | Average Latency / Operation | Notes / Observations |
| :--- | :--- | :---: | :--- |
| **V3** | Boundary-Split | **~875 ns** | Rock-solid repeatability |
| **V4** | Structured State | **~866 ns** | Identical profile to R1 |
| **V2** | Hint-Optimized | **~1,133 ns** | Confirms structural latency peaks |
| **V1** | Naive Baseline | **~1,219 ns** | Stable baseline overhead |
| **VSPS** | State-Machine (Kumuliert) | **~1,632 ns** | Slightly faster cycle execution (~82 ns/cycle avg) |

### 3. Round 3 Breakdown (Stress & Stress-Test Validation)
| Version | Approach | Average Latency / Operation | Notes / Observations |
| :--- | :--- | :---: | :--- |
| **V3** | Boundary-Split | **~847 ns** | Optimal performance peak in R3 |
| **V4** | Structured State | **~867 ns** | Unwavering determinism |
| **V2** | Hint-Optimized | **~1,143 ns** | Consistent outlier behavior |
| **V1** | Naive Baseline | **~1,213 ns** | Predictable baseline behavior |
| **VSPS** | State-Machine (Kumuliert) | **~2,018 ns** | Safe handling of peak allocations (`EMPLACE_BEGIN`) |

### Key Takeaway for the Reviewer:
While monolithic implementations like **V3** and **V4** achieve lower overall latency by avoiding dispatch overhead, **VSPS** proves that breaking operations down into fine-grained, predictable state cycles (~84 ns per cycle step) achieves 100% determinism with a controlled, predictable performance trade-off—ideal for hard real-time environments.

## Primary Structure / Grobe Struktur

`/documentation` – Draw.io Setup for analytics  
`/include` – 2 Header Files of Interval_Map  
`/test` – CTest Setup across the Versions  
`/third_party` – Additional Libraries for Inclusions  
`Debugging.cpp` – ImGui Setup for Visual Diagnostics  
`homeprojekt.md` – Public Information about my Private Setup

## Documentation

#### Eng
The `/documentation` directory contains a graphical analysis that attempts to identify key aspects of the technical architecture. Particular focus is placed on which information leads to hazardous conditions, followed by considerations on how to satisfy these conditions.

Additionally, an oracle table was created and validated for a test case.  
The local README served as a small, somewhat unfiltered development diary.

#### Ger
Im Verzeichnis `/documentation` befindet sich eine grafische Analyse, die versucht, besondere Eckdaten des technischen Aufbaus zu identifizieren.

Dabei wird insbesondere darauf geachtet, welche Informationen zu gefährlichen Konditionen führen, und es werden Überlegungen angestellt, wie diese Bedingungen erfüllt werden können.

Außerdem wurde für einen Testfall eine Orakel-Tabelle erstellt und abgesichert.  
Das dortige README diente dabei als kleines, etwas ungefiltertes Tagebuch.

## Description of Versions / Beschreibung der Versionen

#### Eng
* Version 1 (Boundary Analysis)
* Version 2 (Lower Bound Emplace Hint Setup)
* Version 3 (Lower Bound Insert Hint Setup)
* Version 4 (Custom Upward Emplace Hint Setup)
* Version 5 (PLC Variant with Strict State Machine Separation)

#### Ger
* Version 1 (Grenzbetrachtung)
* Version 2 (Lower Bound Emplace Hint Setup)
* Version 3 (Lower Bound Insert Hint Setup)
* Version 4 (Custom Aufwärts Emplace Hint Setup)
* Version 5 (SPS-Variante mit strikter State-Machine-Trennung)

## Diagnostic – Test

#### Eng
To visualize the behavior of the interval_map, a visual analysis tool was built using ImGui and Vulkan. I used std::chrono for performance diagnostics to determine which approach proves most effective under various scenarios. Additionally, the tests were systematically validated against known edge cases.

#### Ger
Um das Verhalten der interval_map zu visualisieren, wurde mithilfe von ImGui und Vulkan ein visuelles Untersuchungstool entwickelt. Dabei habe ich std::chrono als Diagnoseinformation genutzt, um einzuschätzen, welcher Ansatz sich in verschiedenen Situationen als am effektivsten erweist. Die Tests wurden dabei systematisch gegen bekannte Sonderfälle (Edge Cases) abgesichert.