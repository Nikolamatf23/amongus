# Red Sprout — skin od 50 × 50 px

Crveni član posade sa zelenim listićem. PNG slike imaju providnu pozadinu.

Svaka pojedinačna poza je tačno 50 × 50 px:

- `idle.png`: stajanje, pogled udesno.
- `walk_01.png`, `walk_02.png`, `walk_03.png`: tri poze kretanja udesno.
- `front.png`: pogled spreda.
- `back.png`: pogled otpozadi.
- `wave.png`: mahanje.
- `jump.png`: skok.

`spritesheet.png` je 200 × 100 px: 4 kolone i 2 reda. Redosled je kao u gornjoj listi, sleva nadesno, pa sledeći red.

`walk_2x2.png` je 100 × 100 px: idle, walk_01, walk_02, walk_03, sleva nadesno, pa sledeći red. Početni interval za animaciju: 150 ms po frejmu.

Za izdvajanje frejma iz glavne slike u Qt-u:

```cpp
QPixmap sheet("resources/players/red_sprout/spritesheet.png");
QPixmap frame = sheet.copy((index % 4) * 50, (index / 4) * 50, 50, 50);
```

Putanja u ovom primeru je relativna u odnosu na radni direktorijum aplikacije. Za ugrađivanje u aplikaciju dodaj željene PNG fajlove u `resources.qrc` i koristi odgovarajuću `:/players/` putanju.

Postojeći `PlayerItem.cpp` izdvaja frejmove od 25 × 25 px i crta ih na 24 × 24 px. Za ovaj paket koristi izdvajanje od 50 × 50 px i podesi prikaz na željenu veličinu. Postojeći resursi i ponašanje igre nisu menjani.

`preview.png` je uvećani pregled. `source/original.png` je izvor visoke rezolucije, a `source/prompt.txt` sadrži opis korišćen za generisanje ugrađenim alatom za slike. Finalni PNG fajlovi su izvezeni smanjivanjem i deljenjem izvornog atlasa uz očuvanu providnost.
