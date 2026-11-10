#include "network/ClientController.h"
#include "network/Client.h"
#include "game/GameState.h"

#include <QJsonArray>

ClientController::ClientController(Client* client,
                                   GameState* state,
                                   QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_state(state)
{
    connect(m_client, &Client::connected,
            this, &ClientController::onConnected);
    connect(m_client, &Client::disconnected,
            this, &ClientController::onDisconnected);
    connect(m_client, &Client::messageReceived,
            this, &ClientController::onMessage);
    connect(m_client, &Client::errorOccurred,
            this, &ClientController::onError);
}

// ------------------------------------------------------------ veza

void ClientController::connectToServer(const QString& host,
                                       quint16 tcpPort,
                                       const QString& name)
{
    m_playerName = name;          // Join šaljemo tek kad TCP bude gotov
    m_myId = -1;
    m_nearTaskId = -1;
    m_killTargetId = -1;
    m_canEmergency = false;
    m_client->connectToHost(host, tcpPort);
}

void ClientController::disconnectFromServer()
{
    m_client->disconnectFromHost();
}

void ClientController::onConnected()
{
    sendMessage(MessageType::Join, {{"name", m_playerName}});
}

void ClientController::onDisconnected()
{
    emit connectionLost();
}

void ClientController::onError(const QString& text)
{
    emit errorOccurred(text);
}

// ------------------------------------------------------------ akcije

void ClientController::requestStart()
{
    sendMessage(MessageType::StartGame);
}

void ClientController::sendMoveInput(const QPointF& dir)
{
    // Client sam dodaje playerId, udpToken, phaseId i tick.
    sendMessage(MessageType::MoveInput, {{"dx", dir.x()}, {"dy", dir.y()}});
}

void ClientController::chooseSkin(int skinId)
{
    sendMessage(MessageType::ChooseSkin, {{"skinId", skinId}});
}

void ClientController::requestKill()
{
    if (m_killTargetId < 0) {
        return;
    }
    sendMessage(MessageType::KillRequest, {{"targetId", m_killTargetId}});
    m_killTargetId = -1;   // sprecava dupli klik pre sledeceg snapshota
}

bool ClientController::canKill() const { return m_killTargetId >= 0; }

bool ClientController::canReport() const { return m_nearBodyId >= 0; }

void ClientController::reportBody()
{
    if (m_nearBodyId < 0) {
        return;
    }
    sendMessage(MessageType::ReportBody, {{"bodyId", m_nearBodyId}});
    m_nearBodyId = -1;   // sprecava dupli klik
}

void ClientController::requestEmergency()
{
    sendMessage(MessageType::EmergencyMeeting);
}

void ClientController::completeTask(int taskId)
{
    sendMessage(MessageType::TaskCompleted, {{"taskId", taskId}});
}

void ClientController::castVote(int targetId)
{
    sendMessage(MessageType::Vote, {{"targetId", targetId}});
}

void ClientController::sendChat(const QString& text)
{
    const QString t = text.trimmed();
    if (t.isEmpty()) {
        return;
    }
    sendMessage(MessageType::ChatMessage, {{"text", t}});
}

void ClientController::sendMessage(MessageType type, const QJsonObject& content)
{
    m_client->send(Message(type, content));
}

// ------------------------------------------------------------ stanje

const GameState* ClientController::state() const { return m_state; }
int  ClientController::myId() const              { return m_myId; }
int ClientController::mySkinId() const
{
    const Player* me = m_state->player(m_myId);
    return me ? me->skinId() : 1;
}
bool ClientController::isConnected() const       { return m_client->isConnected(); }
int  ClientController::nearTaskId() const        { return m_nearTaskId; }
int ClientController::killTargetId() const           { return m_killTargetId; }
bool ClientController::canCallEmergency() const  { return m_canEmergency; }
int  ClientController::phaseSeconds() const      { return m_phaseSeconds; }

// ------------------------------------------------------------ prijem

void ClientController::onMessage(const Message& msg)
{
    // get_messageContent() nije const, zato radimo kopiju poruke.
    Message incoming = msg;
    const QJsonObject content = incoming.get_messageContent();

    switch (msg.type()) {

    case MessageType::JoinAccepted:
        // Client je već obradio playerId/udpToken; mi samo pamtimo svoj id.
        m_myId = m_client->myId();
        break;

    case MessageType::LobbyUpdate:
        // Sadržaj = GameState::toJson() u fazi Lobby (lista igrača).
        m_state->fromJson(content);
        emit lobbyChanged();
        break;

    case MessageType::StartGame:
        // Sadržaj = GameState::toJson() sa ulogama i taskovima.
        m_state->fromJson(content);
        emit stateChanged();
        emit phaseChanged(m_state->phase());
        break;

            case MessageType::StateSnapshot:
            m_state->fromJson(content, false);
            m_nearBodyId   = content.value("nearBodyId").toInt(-1);
            m_nearTaskId   = content.value("nearTaskId").toInt(-1);
            m_killTargetId = content.value("killTargetId").toInt(-1);
            m_canEmergency = content.value("canEmergency").toBool(false);
            emit stateChanged();
            break;

    case MessageType::PositionSnapshot:
        // UDP: menja samo pozicije, nikad uloge, alive ili taskove.
        for (const QJsonValue& v : content.value("players").toArray()) {
            const QJsonObject pj = v.toObject();
            if (Player* p = m_state->player(pj.value("id").toInt(-1))) {
                p->setPos(QPointF(pj.value("x").toDouble(),
                                  pj.value("y").toDouble()));
            }
        }
        emit stateChanged();
        break;

    case MessageType::PhaseChanged: {
        const auto ph = static_cast<GamePhase>(content.value("phase").toInt());
        m_state->setPhase(ph);
        m_phaseSeconds = content.value("seconds").toInt(0);   // npr. 180
        emit phaseChanged(ph);
        break;
    }

    case MessageType::ChatMessage:
        emit chatReceived(ChatMessage::fromJson(content));
        break;

    case MessageType::VoteResult:
        emit voteResult(content.value("ejectedId").toInt(-1),
                        content.value("wasImpostor").toBool(false));
        break;

    case MessageType::GameOver:
        // Server tu šalje i završno stanje sa otkrivenim ulogama.
        if (content.contains("players")) {
            m_state->fromJson(content);
        }
        emit gameOver(static_cast<Winner>(content.value("winner").toInt()));
        break;

    default:
        break;
    }
}