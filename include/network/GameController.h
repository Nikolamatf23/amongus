#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>
#include <QTimer>

#include "network/Message.h"
#include <QElapsedTimer>

class Server;
class GameState;
class VotingSession;

class GameController : public QObject
{
    Q_OBJECT

public:
    explicit GameController(
        Server* server,
        GameState* state,
        QObject* parent = nullptr
    );

    ~GameController() override;

    void startGame();

private slots:
    void onMessage(int playerId, const Message& msg);
    void onPlayerLeft(int playerId);

    void tick();
    void sendPositions();
    void endMeeting();
    void onMeetingTimeout();

private:
    static constexpr double TASK_RANGE         = 70.0;  // px
    static constexpr double KILL_RANGE         = 65.0;  // px
    static constexpr double KILL_COOLDOWN      = 25.0;  // sekundi

    static constexpr double MOVE_SPEED         = 180.0; // px/s
    static constexpr double DISCUSSION_TIME    = 30.0;  // sekundi
    static constexpr double VOTING_TIME        = 45.0;  // sekundi

    static constexpr double EMERGENCY_RANGE    = 70.0;  // px
    static constexpr double EMERGENCY_COOLDOWN = 30.0;  // sekundi

    void handleJoin(int playerId, const Message& msg);
    void handleMove(int playerId, const Message& msg);
    void handleKill(int playerId, const Message& msg);
    void handleReport(int playerId, const Message& msg);
    void handleTaskCompleted(int playerId, const Message& msg);
    void handleVote(int playerId, const Message& msg);
    void handleEmergencyMeeting(int playerId, const Message& msg);
    void handleChat(int playerId, const Message& msg);
    void handleEmergency(int playerId, const Message& msg);
    bool canCallEmergency(int playerId);

    void assignRoles();
    void assignTasks();

    void startMeeting(int callerId, int bodyId);
    void checkWin();

    void sendLobbyUpdate();
    void sendSnapshot();
    void sendPhaseChanged();

    Server* m_server;
    GameState* m_state;
    VotingSession* m_voting = nullptr;

    QTimer m_tickTimer;
    QTimer m_snapshotTimer;
    QTimer m_meetingTimer;
    //uvodim i m_tickClock, meri proteklo vreme za računanje dt - vreme koje je prošlo od
    //prethodnog poziva tick(), izraženo u sekundama.
    QElapsedTimer m_tickClock;

    quint32 m_positionSnapshotTick = 0;

    double m_emergencyCooldown;


};

#endif // GAMECONTROLLER_H