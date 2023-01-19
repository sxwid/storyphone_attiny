# storyphone_attiny
Wählscheibentelefon MP3 Player

Code und Hardware für einen Standalone Player. Funktionen und Details:
- Speisung 24VDC (Ein 6W Netzteil reicht)
- 2x Schaltregler um 5V für das MP3 Modul und den Attiny zu generieren (getrennt wegen Störungen des MP3 Moduls)
- Ausgabe Leitungston 
- Detektion der Gabelzustände und Wählimpulse über Optokoppler
- FSM mit Statusindikation über LED
- USB Stick (FAT32) mit Dateien im Rootfolder:
  - 0001.mp3 für Zahl 1
  - 0002.mp3 für Zahl 2 
  -  etc 
  - 0009.mp3 für Zahl 9
  - 0010.mp3 für Zahl 0

Signalverlauf und FSM Aufbau

![](https://github.com/sxwid/storyphone_attiny/blob/b8405406c7edf630e6ffa7b5f0768eefb24f5b94/impulse.png)

Hier das Schema für die Autophon Geräte

![](https://raw.githubusercontent.com/sxwid/storyphone/master/autophon.png)

Bei Standardkabelbelegung gilt:
- a Draht weiss (a)
- b Draht blau (b) 
- c Draht türkis (Wecker)
- d Draht violett

Da wir den Wecker nicht benötigen können der weisse und der blaue Draht verwendet werden.
