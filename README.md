# Mutli Version Interval Map
## Overview/Überblick  
  
#### Eng  
! Important: The choice to split the blocks stems from the fact that I've found thinking in my native language significantly helps in identifying edge cases and danger zones. Because of this, you might still encounter some German context or comments here and there—either because I couldn't find an ideal translation at the moment or simply left them as is while focusing fully on the core logic.

#### Ger
Dieses Repository dient als Entwicklungstagebuch und dokumentiert die Entwicklung der interval_map sowie den Aufbau der verschiedenen Versionen. 
Wichtiger Hinweis zum Sprachen-Mix:
Meine primäre Analysesprache ist Deutsch. Zwar habe ich kein grundsätzliches Problem mit Englisch, aber ich bin darin etwas eingerostet, weshalb ich dafür oft länger brauche, als mir lieb ist. Zudem habe ich die Erfahrung gemacht, dass das Denken in der Muttersprache extrem dabei hilft, Corner/Edge Cases (Grenzfälle) und Gefahrenzonen (Danger Zones) zielsicher zu identifizieren und zu lösen.

Aus diesem Grund wirst du hier im Repository noch deutschen Kontext, Kommentare oder Begriffe finden – teils, weil mir in dem Moment keine ideale englische Übersetzung eingefiel, oder weil es schlichtweg so stehen geblieben ist, während mein Fokus auf der mathematischen und logischen Korrektheit lag. Ich bitte um Verzeihung, falls dadurch anfangs die ein oder andere Namensgebung etwas unkonventionell wirkt.

(Hinweis zur Dokumentation: Dieser Text wurde mit unterstützender KI-Assistenz geprüft; die englische Fassung ist dabei als kompakte Zusammenfassung gedacht, während der deutsche Text hier die tieferen Details und Hintergründe enthält.)


  
## Primary Structure/ Grobe Struktur
`/documentation`          - Draw io Setup for analytics  
`/include`                       - 2 Header Files of Interval_Map  
`/test`                             - CTest Setup Across of the Versions  
`/third_party`                  - Additional Libarys for Inclusions  
`Debugging.cpp`            - ImGui Setup for Visual Diagnostic                         
`homeprojekt.md`          - Public Information of my Privat Setup
  

## Documentation  
#### Eng
The `/documentation` directory contains a graphical analysis that attempts to identify key aspects of the technical architecture. Particular focus is placed on which information leads to hazardous conditions, followed by considerations on how to satisfy these conditions. 

Additionally, an oracle table was created and validated for a test case. 
The local README served as a small, somewhat unfiltered development diary.

#### Ger
Im Verzeichnis `/documentation` befindet sich eine Grafische Analyse, welche versucht besondere Eckdaten des Technischen Aufbau zu Identifizieren.  
Dabei wird im besonderen darauf geachtet, welche Informationen zu gefährliche Kondidtionen führt und zusätzlich dann Überlegungen angestellt werden, um die Bedingungen zu erfüllen.  
Außerdem wurde für ein Testcase eine Orakel Tabelle erstellt und dort abgesichert.
Das dortige Readme diente dabei als kleinen etwas ungefilterte Tagebuch. 

  

## Description of Version/ Beschreibung der Versionen
#### Eng
* Version 1 (Boundary Analysis)
* Version 2 (Lower Bound Emplace Hint Setup)
* Version 3 (Lower Bound Insert Hint Setup)
* Version 4 (Custom Upward Emplace Hint Setup)
* Version 5 (PLC Variant with Strict State Machine Separation)


#### Ger
* Version 1 (Grenz Betrachtung)  
* Version 2 (Lower Bound Emblace hint Setup)  
* Version 3 (Lover Bound Insert hint Setup)  
* Version 4 (Custom Aufwärts Emplace hint Setup)
* Version 5 (SPS Variante Stricte State Machine Trennung) 
  
 
  
  
  
## Diagnostic - Test  
  
#### Eng  
To visualize the behavior of the interval_map, a visual analysis tool was built using ImGui and Vulkan. I used std::chrono for performance diagnostics to determine which approach proves most effective under various scenarios. Additionally, the tests were systematically validated against known edge cases.

#### Ger
Um das Verhalten der interval_map zu visualisieren, wurde mithilfe von ImGui und Vulkan ein visuelles Untersuchungstool entwickelt. Dabei habe ich std::chrono als Diagnoseinformation genutzt, um einzuschätzen, welcher Ansatz sich in verschiedenen Situationen als am effektivsten erweist. Die Tests wurden dabei systematisch gegen bekannten Sonderfälle (Edge Cases) abgesichert.