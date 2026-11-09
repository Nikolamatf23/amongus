# 16 različitih skinova i njihove dead verzije

Pogledaj `preview_16_skins.png`: svaki par prikazuje živi skin levo i odgovarajući dead desno. Poze su poređane u četiri reda, sleva nadesno.

Svaki folder `01_sakura` do `16_space_ranger` sadrži `alive.png` i `dead.png`, oba **50 × 50 px**, sa providnom pozadinom. `pair.png` je pregled para, **100 × 50 px**.

Za animaciju kretanja svaki folder sadrži i `idle.png`, `walk_01.png`, `walk_02.png`, `walk_03.png`, sve **50 × 50 px**. `idle.png` je kopija postojećeg `alive.png`; ostala tri frejma imaju različite položaje nogu. `walk_2x2.png` je gotova mreža od četiri frejma, **100 × 100 px**, redom: idle i walk_01 u gornjem redu, walk_02 i walk_03 u donjem. `walk_strip.png` je isti redosled u jednom redu, **200 × 50 px**. Pregled svih 16 animacija nalazi se u `preview_16_walks.png`.

Skinovi po redosledu: Sakura, Pirate, Wizard, Chef, Robot, Bumblebee, Cactus, Firefighter, Mushroom, Royal, Ninja, Diver, Frost, Dragon, Detective i Space Ranger. Dead verzije prikazuju donju polovinu odela sa belom kosti i odgovarajućim dodatkom pored nje.

Za Qt projekat dodaj izabrane PNG fajlove u `resources.qrc`, na primer sa aliasima `sakura_alive.png`, `sakura_walk.png` i `sakura_dead.png`, pa ih učitaj preko `:/players/` putanje. Za četiri frejma iz `walk_2x2.png` koristi `copy((frame % 2) * 50, (frame / 2) * 50, 50, 50)` i interval oko **150 ms**. Kada se igrač zaustavi, prikaži `idle.png`; kada umre, prikaži `dead.png`. Za kretanje ulevo možeš da preslikaš frejm horizontalno. Postojeći `PlayerItem.cpp` koristi frejmove od 25 × 25 px, pa prilagodi izdvajanje na 50 × 50 px kada budeš povezivao ove skinove sa igrom.

Izvorne slike visoke rezolucije i tekstovi korišćeni za generisanje ugrađenim alatom za slike nalaze se u `source/`. `build_assets.sh` pravi žive i dead PNG fajlove, a `build_walk.sh` pravi animacione frejmove i pregled.
