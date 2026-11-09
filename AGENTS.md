# mp3encoder

Dieses Projekt enthält eine Anwendung zum Encodieren von Audiodateien im MP3-Format.
Das User-Interface basiert auf Qt6. Die Dekodierung und Encodierung verwendet bisher meine Libraray ppl7 (pplib).

Die Anwendung läuft unter Windows und Linux.

# Refactoring
- Umstellung von ppl7 auf pplib8 (Migration)
- Umstellung auf CMake
- C++20 als Standard verwenden
- Unterstützung anderer Audio-Formate als Ziel-Format

## Migration auf pplib8
pplib8 ist die neuere Version von ppl7. Sie verwendet bereits cmake und ist im Homeverzeichnis des Benutzers installiert.
Siehe `~/lib64/cmake/pplib8/` für die CMake-Konfigurationsdateien von pplib8.

Das Projekt "pplib8", ist eine plattformübergreifende C++-Bibliothek, die auf verschiedenen Plattformen (Windows MingW64/MSys, Linux, FreeBSD) getestet wird und sowohl auf Desktop- als auch in Teilen auf Mikrocontroller-Umgebungen läuft.

pplib8 verwendet als Namespace `pplib` anstelle von `ppl7`. Die meissten Klassen und Funktionen sind gleich geblieben, die API wurde aber an einigen Stellen erweitert, und an wenigen Stellen geändert.

Gravierenste Ännderungen:
- Namespace: pplib
- Statt weniger zentraler Header-Dateien gibt es jetzt viele spezialisierte Header-Dateien, zum Beispiel `pplib/types/string.h`. Die zentralen Header-Dateien wie `pplib.h` sind noch vorhanden, sollten aber möglichst nicht mehr direkt inkludiert werden.
- Dir-Klasse wurde stark überarbeitet, insbesondere die Itteratoren sind jetzt standardkonforme C++-Iteratoren.
- File-Klasse: map()-Methoden wurden überarbeitet (tryMap) 
- Audio-Decoder wurde leicht angepasst, getSamples wirft nicht mehr, sondern gibt auch im Fehlerfall 0 zurück.
- Encoder ist größtenteils übernommen worden, ist aber noch ungetestet und könnte Fehler enthalten.


