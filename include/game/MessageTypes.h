#ifndef GAME_MESSAGE_TYPES_H
#define GAME_MESSAGE_TYPES_H

enum class MessageType
{
    Join = 0,              // Zahtev za ulazak u partiju.
    JoinAccepted = 1,      // Potvrda ulaska u partiju.
    LobbyUpdate = 2,       // Ažuriranje podataka u lobiju.
    StartGame = 3,         // Poruka za pokretanje partije.
    MoveInput = 4,         // Zahtev za kretanje igrača.
    StateSnapshot = 5,     // Snimak trenutnog stanja partije.
    KillRequest = 6,       // Zahtev za eliminaciju igrača.
    ReportBody = 7,        // Prijava tela.
    TaskCompleted = 8,     // Obaveštenje da je zadatak završen.
    PhaseChanged = 9,      // Promena faze partije.
    ChatMessage = 10,      // Poruka u četu.
    Vote = 11,             // Glas igrača.
    VoteResult = 12,       // Rezultat glasanja.
    GameOver = 13,         // Kraj partije.
    UdpRegister = 14,      // Zahtev za registraciju UDP veze.
    UdpRegistered = 15,    // Potvrda UDP registracije.
    PositionSnapshot = 16, // Ažuriranje pozicija igrača.
    Invalid = 17           // Oznaka neuspešnog parsiranja poruke.
};

#endif // GAME_MESSAGE_TYPES_H
