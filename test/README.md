# Aufbau der Testumgebung

Fakt 1 - Jede Berechnung wird gegen std::chrono::steady_clock gelegt


Cases

## Ungültige Intervall
    Es muss immer ein Intervall vorliegen
    Ein Negativ Intervall existiert nicht.

## Vollständige Intervall Schreibweise (Kanozität)
    Das gleiche Value darf nicht hintereinander stehen
    Value "endet" immer mit Default Value

## Key Operator
    Key darf nur über den Sotierungsoperator < laufen

## Value Operator
    Value darf nur über den Vergleich operator laufen

## Strong Expetion
    Das einfügen von Keybegin bzw Keyend bei einem Catch fall, darf kein Änderung durchführe