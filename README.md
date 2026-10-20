# Mutli Version Interval Map
##Overview/Überblick  
  
####Eng  
! Important the selection to Split the Blocks belongs to the Situation, that i identified several time, to think in the Motherlanguage supports surely the ability to identify Corner/Edge Cases and Danger Zones. Thats the Reason, when you still will find a set German Context, its belongs to the Point that its possible, that i am not able at this moment to find the best way to translate it or i forgot that this is still there !

####Ger
Diese Repository dient als Entwicklungstagebuch.  
Es dokumentiert die Entwicklung der Interval_map und der Aufbau der verschiedenen Versionen. 

  
##Primary Structure/ Grobe Struktur
/documentation          - Draw io Setup for analytics  
/include                       - 2 Header Files of Interval_Map  
/test                             - CTest Setup Across of the Versions  
/third_party                  - Additional Libarys for Inclusions  
Debugging.cpp            - ImGui Setup for Visual Diagnostic                         
homeprojekt.md          - Public Information of my Privat Setup
  

##Documentation  
####Eng

####Ger
Im Verzeichnis /documentation befindet sich eine Grafische Analyse, welche versucht besondere Eckdaten des Technischen Aufbau zu Identifizieren.  
Dabei wird im besonderen darauf geachtet, welche Informationen zu gefährliche Kondidtionen führt und zusätzlich dann Überlegungen angestellt werden, um die Bedingungen zu erfüllen.  

  
##Description of Version/ Beschreibung der Versionen
####Eng

####Ger
Version 1 (Grenz Betrachtung)  
Version 2 (lower_bound -> aufwärts)  
Version 3 (lower_bound -> abwärts)  
Version 4 (upper_bound ->aufwärts)
Version 5 (upper_bound ->abwärts)  
Version 6 (Custom Iteration -> aufwärts)  
Version 7 (Custom Iteration -> abwärts)  
  
Version 8 (SPS Synchron Ansatz, aufwärts)  
  
  
  
##Diagnostic - Test  
  
####Eng  
  
####Ger
Um das Verhalten der Interval Map zu verstehen, ist mithilfe von Imgui für Vulkan eine Visuelle Untersuchung erstellt.  
Dabei ist libcycle als Diagnose Information genutzt, um einzuschätzen welcher Weg unter Verschiedene Situationen sich als effektivsten zeigt.  
  
Die Test sind dabei gegen die bekannten Sonderfälle gelegt.