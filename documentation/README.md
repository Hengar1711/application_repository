# Diary Documentation

1. Der Weg zur Wahrheit: Die Orakel-Tabelle wurde angefangen, weil man mathematische Logik oft erst dann verlässlich auf absolute Wahrheit prüfen kann, wenn der Zwischenschritt – beispielsweise über eine visuelle Untersuchung – erarbeitet wurde.

2. Runde 1 – Nur Ranges: Die Tabelle startete zunächst damit, nur die Intervalle („Ranges“) einzuzeichnen, um die Gesamtanzahl der Intervalle zu erfassen. Dabei hatten sich im ersten Durchlauf direkt Fehler an den key_end-Grenzen eingeschlichen.

3. Runde 2 – Modifizierte Intervalle: In der zweiten Runde wurde zusätzlich markiert, welches Intervall konkret angepasst oder angefasst wurde.

4. Runde 3 – Übernahme in die Test Suite: In Runde 3 wurden die Intervalle schließlich in die Test Suite übernommen, um automatisiert zu prüfen, ob sie korrekt sind. Dabei fielen noch einmal gezielt key_end-Fehler auf.

5. Der Einsatz der Gummi-Ente: Ich habe während des Prozesses bewusst eine „Gummi-Ente“ in Form von KI-/LLM-Systemen genutzt.

6. Kritische Distanz: Hätte ich mich blind darauf verlassen, ohne selbst gegenzuzuprüfen, hätte ich zu 90 % nichts Vernünftiges geschafft und wäre ziemlich aufgeschmissen gewesen.

7. Gezielter Einsatz: Aus diesem Grund habe ich das Tool primär zur Formulierung/Modulation von Texten oder für recht einfache Aufgaben genutzt, bei denen ich um Hilfe gebeten habe.

8. Der Realitätscheck (12.11.): Tatsächlich wurde durch die Visualisierung in assign1 am 12.11. noch ein Fehler gefunden, der in den vorherigen Versionen so gar nicht existiert hatte – ein Beweis, dass visuelle Kontrolle durch nichts zu ersetzen ist.

9. Geständnis: Ich schreibe dieses Tagebuch gerade nachträglich, weil mein Fokus während des Codings zu oft komplett woanders lag (beim reinen Problemlösen).

10. Die Ironie des Prozesses: Ich war durchaus amüsiert darüber, dass ich eine sauber laufende Version 4 hatte und bei der Überleitung zur 5. Version trotzdem noch in einer eigentlich leichten Konvertierung gestolpert bin ^^.
    
11. Dabei erinnere ich mich auch noch, dass ich länger gebraucht hatte, die Realität von lower_bound richtig zu verstehen. Als ich mir das dann mithilfe des Memory-Speichers visualisiert habe, machte es Sinn: Lower Bound als linker Rand, in dem der Speicher sein kann, gegenüber dem rechten Rand, wo der Speicher garantiert nicht mehr is