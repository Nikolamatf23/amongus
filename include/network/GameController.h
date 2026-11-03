#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>
#include <QTimer>

#include "network/Message.h"

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

private:
    void handleJoin(int playerId, const Message& msg);
    void handleMove(int playerId, const Message& msg);
    void handleKill(int playerId, const Message& msg);
    void handleReport(int playerId, const Message& msg);
    void handleTaskCompleted(int playerId, const Message& msg);
    void handleVote(int playerId, const Message& msg);
    void handleChat(int playerId, const Message& msg);

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
};

#endif // GAMECONTROLLER_H