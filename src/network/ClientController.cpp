#include "network/ClientController.h"
#include "network/Client.h"

ClientController::ClientController(Client* client, QObject* parent)
    : QObject(parent),
    m_client(client)
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
                                       const QString& playerName)
{
    // Ime pamtimo jer Join saljemo tek kad TCP veza bude gotova.
    m_playerName = playerName;
    m_client->connectToHost(host, tcpPort);
}

void ClientController::disconnectFromServer()
{
    m_client->disconnectFromHost();
}

bool ClientController::isConnected() const
{
    return m_client->isConnected();
}

int ClientController::myId() const
{
    return m_client->myId();
}

void ClientController::onConnected()
{
    sendMessage(MessageType::Join, {{"name", m_playerName}});
    emit connectedToServer();
}

void ClientController::onDisconnected()
{
    emit disconnectedFromServer();
}

void ClientController::onError(const QString& text)
{
    emit errorOccurred(text);
}

// ------------------------------------------------------------ akcije

void ClientController::sendMove(double dx, double dy)
{
    // Client sam dodaje playerId, udpToken, phaseId i tick.
    sendMessage(MessageType::MoveInput, {{"dx", dx}, {"dy", dy}});
}

void ClientController::requestKill(int targetId)
{
    sendMessage(MessageType::KillRequest, {{"targetId", targetId}});
}

void ClientController::reportBody(int bodyId)
{
    sendMessage(MessageType::ReportBody, {{"bodyId", bodyId}});
}

void ClientController::completeTask(int taskId)
{
    sendMessage(MessageType::TaskCompleted, {{"taskId", taskId}});
}

void ClientController::sendVote(int targetId)
{
    sendMessage(MessageType::Vote, {{"targetId", targetId}});
}

void ClientController::sendChat(const QString& text)
{
    if (text.trimmed().isEmpty()) {
        return;
    }
    sendMessage(MessageType::ChatMessage, {{"text", text}});
}

void ClientController::sendMessage(MessageType type, const QJsonObject& content)
{
    m_client->send(Message(type, content));
}

// ------------------------------------------------------------ prijem

void ClientController::onMessage(const Message& msg)
{
    // get_messageContent nije const, zato radimo kopiju poruke.
    Message incoming = msg;
    const QJsonObject content = incoming.get_messageContent();

    switch (msg.type()) {
    case MessageType::JoinAccepted:
        // Client je vec obradio playerId/udpToken, mi samo javljamo dalje.
        emit joined(m_client->myId());
        break;

    case MessageType::LobbyUpdate:
        emit lobbyUpdated(content);
        break;

    case MessageType::StartGame:
        emit gameStarted(content);
        break;

    case MessageType::StateSnapshot:
        emit stateSnapshotReceived(content);
        break;

    case MessageType::PositionSnapshot:
        emit positionsReceived(content);
        break;

    case MessageType::PhaseChanged:
        emit phaseChanged(content.value("phaseId").toInt(0), content);
        break;

    case MessageType::ChatMessage:
        emit chatReceived(content.value("senderId").toInt(-1),
                          content.value("text").toString());
        break;

    case MessageType::VoteResult:
        emit voteResultReceived(content);
        break;

    case MessageType::GameOver:
        emit gameOver(content);
        break;

    default:
        break;
    }
}
