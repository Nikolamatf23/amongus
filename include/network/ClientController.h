#ifndef CLIENTCONTROLLER_H
#define CLIENTCONTROLLER_H

#include <QObject>
#include <QPointF>
#include <QString>
#include <QJsonObject>

#include "game/ChatMessage.h"
#include "game/GameTypes.h"
#include "network/Message.h"

class Client;
class GameState;

// Klijentski kontroler (dijagram: ClientController).
// Ništa ne odlučuje: UI-jeve akcije pretvara u Message i šalje preko Client-a,
// a poruke sa servera upisuje u lokalnu kopiju GameState-a i javlja UI-ju
// signalima. Ne zna za transport (TCP/UDP) - to radi Client.
class ClientController : public QObject
{
    Q_OBJECT

public:
    // Client i GameState NE posedujemo (vlasnik je MainWindow).
    ClientController(Client* client,
                     GameState* state,
                     QObject* parent = nullptr);

    // ---- veza ----
    // Otvara TCP; Join se šalje automatski kad se veza uspostavi.
    void connectToServer(const QString& host,
                         quint16 tcpPort,
                         const QString& name);
    void disconnectFromServer();

    // ---- akcije igrača (UI ih poziva) ----
    void requestStart();                    // samo host (server proverava)
    void sendMoveInput(const QPointF& dir); // ide preko UDP-a
    void requestKill();
    void reportBody();
    void chooseSkin(int skinId);
    bool canReport() const;   // blizu tela
    bool canKill() const;
    void requestEmergency();                // emergency dugme u cafeteriji
    void completeTask(int taskId);
    void castVote(int targetId);            // -1 = Skip
    void sendChat(const QString& text);

    // ---- stanje za UI ----
    const GameState* state() const;
    int  myId() const;
    int  mySkinId() const;
    bool isConnected() const;

    // Server računa, klijent samo prikazuje (stiže u StateSnapshot-u).
    int  nearTaskId() const;        // -1 ako nema taska u dometu
    int killTargetId() const;
    bool canCallEmergency() const;  // blizu stola i još nije iskorišćeno
    int  phaseSeconds() const;      // trajanje faze (npr. 180 za glasanje)

signals:
    void lobbyChanged();
    void stateChanged();
    void phaseChanged(GamePhase ph);
    void chatReceived(const ChatMessage& msg);
    void voteResult(int ejectedId, bool wasImpostor);
    void gameOver(Winner w);
    void connectionLost();
    void errorOccurred(const QString& text);   // za neuspelo povezivanje

private slots:
    void onConnected();
    void onDisconnected();
    void onMessage(const Message& msg);
    void onError(const QString& text);

private:
    void sendMessage(MessageType type,
                     const QJsonObject& content = QJsonObject());

    Client*    m_client;
    GameState* m_state;
    int        m_myId = -1;
    QString    m_playerName;

    int  m_nearTaskId = -1;
    int m_nearBodyId = -1;
    int m_killTargetId = -1;
    bool m_canEmergency = false;
    int  m_phaseSeconds = 0;
};

#endif // CLIENTCONTROLLER_H