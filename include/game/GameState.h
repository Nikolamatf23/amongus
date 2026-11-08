#ifndef GAMESTATE_H
#define GAMESTATE_H

// ============================================================================
//  GameState
//  Celo stanje partije: igraci, mrtva tela, faza, brojac taskova.
//  Ne zna ni za UI ni za mrezu.
//   - SERVER (GameController) ga menja kao istinu i salje toJson().
//   - KLIJENT (ClientController) drzi kopiju i azurira je sa fromJson().
//
//  Dijagram:
//    - m_players: QMap<int, Player>   - m_bodies: QVector<DeadBody>
//    - m_phase: GamePhase             - m_map: GameMap*
//    - m_tasksDone: int               - m_tasksTotal: int
//    + addPlayer(p) / removePlayer(id)
//    + player(id): Player*
//    + players(): QList<Player>
//    + alivePlayers(): QList<Player>
//    + addBody(b) / bodies()
//    + clearBodies()
//    + phase() / setPhase(ph)
//    + taskProgress(): double
//    + checkWinCondition(): Winner
//    + toJson(): QJsonObject
//    + fromJson(obj)
//
//  Taskove (Task*) koje igraci imaju brise GameState: u removePlayer(),
//  u destruktoru i u fromJson() kad task nestane iz snapshota.
// ============================================================================

#include "game/DeadBody.h"
#include "game/GameTypes.h"
#include "game/Player.h"

#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QVector>

class GameMap;

class GameState
{
public:
    explicit GameState(GameMap* map = nullptr);
    ~GameState();

    // Zabranjeno kopiranje: dve kopije bi brisale iste taskove.
    GameState(const GameState&) = delete;
    GameState& operator=(const GameState&) = delete;

    void addPlayer(const Player& p);
    void removePlayer(int id);

    Player*       player(int id);
    const Player* player(int id) const;

    QList<Player> players() const;
    QList<Player> alivePlayers() const;

    void              addBody(const DeadBody& b);
    QVector<DeadBody> bodies() const;
    void              clearBodies();

    GamePhase phase() const;
    void      setPhase(GamePhase ph);

    // Udeo zavrsenih taskova posade, 0.0 .. 1.0 (za QProgressBar).
    double taskProgress() const;

    // None dok igra traje; Crewmates ili Impostors kad je kraj.
    Winner checkWinCondition() const;

    QJsonObject toJson() const;
    void        fromJson(const QJsonObject& obj);

    //dodao sam getter jer ne mogu nikako drugacije
    //da dobijem map a treba mi za lokacije taskova u assignTasks
    GameMap* getMap() const;

private:
    QMap<int, Player>  m_players;
    QVector<DeadBody>  m_bodies;
    GamePhase          m_phase = GamePhase::Lobby;
    GameMap*           m_map   = nullptr;
    // mutable: osvezavaju se u const metodama taskProgress()/checkWinCondition()
    mutable int        m_tasksDone  = 0;
    mutable int        m_tasksTotal = 0;
};

#endif // GAMESTATE_H
