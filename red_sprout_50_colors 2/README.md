# Red Sprout: 50 nijansi

Paket sadrži 50 varijanti crvenog skina sa listićem. Položaji, vizir i listić ostaju isti; samo odelo menja boju. Pregled svih nijansi nalazi se u `preview_50_colors.png`, poređan redom po brojevima foldera, sleva nadesno u 10 kolona i pet redova.

Svaki numerisani folder (`01_coral_soft` do `50_navy_normal`) sadrži:

- `idle.png`, `walk_01.png`, `walk_02.png`, `walk_03.png`, `front.png`, `back.png`, `wave.png` i `jump.png`: pojedinačne poze od **50 × 50 px**.
- `spritesheet.png`: iste poze u mreži 4 × 2, ukupno **200 × 100 px**.
- `walk_2x2.png`: četiri frejma za kretanje u mreži 2 × 2, ukupno **100 × 100 px**.

Sve slike su PNG sa providnom pozadinom. Nijanse su poređane po porodicama boja, od svetlije preko obične do tamnije (`soft`, `normal`, `rich`). Poslednja porodica ima dve nijanse kako bi ukupan broj bio tačno 50.

Za upotrebu u Qt projektu, dodaj željenu sliku u `resources.qrc` i učitaj je kao resurs. Iz `spritesheet.png` frejm se izdvaja sa `copy((index % 4) * 50, (index / 4) * 50, 50, 50)`. Za ciklus kretanja koristi `walk_2x2.png` i frejmove širine 50 px.

Varijante su izvedene iz izvornog skina u `../red_sprout/` determinističkom promenom boje odela. Skripta `../generate_color_variants.sh` ponovo pravi ceo paket. Izvorni skin je napravljen ugrađenim alatom za generisanje slika; opis generisanja je u `../red_sprout/source/prompt.txt`.
