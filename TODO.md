- [ ] Kada se dodaju nove vrste taskova, prilagoditi `GameController::assignTasks()` da bira različite vrste i dodeljuje ih igračima na odgovarajućim stanicama.
- [ ] U `handleJoin()` se odbijanje ulaska za sada obrađuje samo sa `return` (pun lobby, partija je već počela ili ime nije ispravno). Dodati tip poruke `JoinRejected` sa razlogom odbijanja, kako bi klijent mogao da ga prikaže korisniku.
- [ ] Dodati interpolaciju pozicija u `GameScreen` radi glatkog kretanja: čuvati primljene pozicije sa vremenom/tickom i između njih računati prikazanu poziciju `PlayerItem` objekata, bez menjanja serverskog stanja u `GameState`.
- [ ] Trebamo da se dogovirmo kako ccemo da radimo za coveka koji umre, sta ce on onda da vidi
- [ ] UI mora da aktivira mogucnost da se kliken dugme use ili kill samo kada smo dovoljno blizu taska, drugog igraca ili dead body-a
- [ ] dodao sam i removePlayer u voting session zbog funkije GameController::onPlayerLeft jer moramo da ga izbacimo iz m_votes i m_eligible
- [ ] Zaštititi `GameState::fromJson()` od nepotpunih snapshot poruka: ako JSON ne sadrži polje `tasks`, ne treba automatski brisati postojeće taskove igrača.

- [ ] Proširiti `assignTasks()` tako da, pored `WireTask`, podržava i druge vrste taskova i dodeljuje ih igračima na odgovarajućim stanicama.

- [ ] Dodati poruku `JoinRejected` koja klijentu vraća razlog odbijanja pridruživanja — puno predvorje, partija je već počela, zauzeto ime ili neispravno ime.
