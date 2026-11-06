#include "network/Server.h"
#include "network/ClientConnection.h"

#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QTcpSocket>
#include <stdexcept>
#include <QUuid>

#include <cmath>
#include <limits>

Server::Server(QObject* parent)
    : QObject(parent)
{
    connect(&m_tcp,
    &QTcpServer::newConnection,
    this,
        &Server::onNewConnection
        );

    connect(
        &m_udp,
        &QUdpSocket::readyRead,
        this,
        &Server::onUdpReadyRead
        );
}

// MainWindow poziva ovo kada host pokrene server.
bool Server::startListening(
    quint16 tcpPort,
    quint16 udpPort
    )
{
    if (!m_udp.bind(QHostAddress::Any, udpPort)) {
        return false;
    }

    if (!m_tcp.listen(QHostAddress::Any, tcpPort)) {
        m_udp.close();
        return false;
    }

    return true;
}

void Server::stop()
{
    m_tcp.close();
    m_udp.close();

    for (const PlayerNetworkSession& session : m_players) {
        if (session.tcpConnection != nullptr) {
            session.tcpConnection->deleteLater();
        }
    }

    m_players.clear();
    m_lastMoveTick.clear();
    m_phaseId = 0;
}
bool Server::trySendWithUdp(
    const Message& msg,
    MessageType type,
    const PlayerNetworkSession& session
    )
{
    if (type != MessageType::PositionSnapshot) {
        return false;
    }

    QByteArray datagram = msg.toBytes();

    if (datagram.size() > 1200) {
        throw std::length_error("Message is too big");
    }

    if (session.udpAddress.isNull()) {
        throw std::invalid_argument("UDP address is not set");
    }

    if (session.udpPort == 0) {
        throw std::invalid_argument("UDP port is not set");
    }

    qint64 bytesSent = m_udp.writeDatagram(
        datagram,
        session.udpAddress,
        session.udpPort
        );

    if (bytesSent != datagram.size()) {
        throw std::runtime_error("UDP failed");
    }

    return true;
}

bool Server::trySendWithTCP(
    const Message& msg,
    MessageType type,
    const PlayerNetworkSession& session
    )
{
    if (type == MessageType::Invalid ||
        type == MessageType::PositionSnapshot) {
        return false;
    }

    if (session.tcpConnection == nullptr) {
        throw std::runtime_error("TCP connection is not set");
    }

    // Kopija je potrebna samo za poruke koje dopunjujemo.
    if (type == MessageType::JoinAccepted ||
        type == MessageType::PhaseChanged) {
        Message outgoing = msg;
        QJsonObject& content = outgoing.get_messageContent();

        if (type == MessageType::JoinAccepted) {
            content["playerId"] = session.tcpConnection->playerId();
            content["udpToken"] = session.udpToken;
            content["serverUdpPort"] =
                static_cast<int>(m_udp.localPort());
        }

        if (type == MessageType::PhaseChanged) {
            content["phaseId"] = static_cast<int>(m_phaseId);
        }

        session.tcpConnection->send(outgoing);
        return true;
    }

    session.tcpConnection->send(msg);
    return true;
}

void Server::broadcast(const Message& msg)
{
    if (msg.type() == MessageType::Invalid) {
        throw std::invalid_argument(
            "Server::broadcast: neispravna poruka"
            );
    }

    // Globalno stanje menjamo jednom, pre petlje.
    if (msg.type() == MessageType::StartGame) {
        m_phaseId = 0;
        m_lastMoveTick.clear();
    }

    if (msg.type() == MessageType::PhaseChanged) {
        ++m_phaseId;
        m_lastMoveTick.clear();
    }

    // session je referenca na trenutni zapis u m_players.
    for (const PlayerNetworkSession& session : m_players) {
        if (msg.type() == MessageType::PositionSnapshot) {
            trySendWithUdp(msg, msg.type(), session);
        }
        else {
            trySendWithTCP(msg, msg.type(), session);
        }
    }
}

void Server::sendTo(
    int playerId,
    const Message& msg
    )
{
    if (msg.type() == MessageType::Invalid) {
        throw std::invalid_argument(
            "Server::sendTo: neispravna poruka"
            );
    }

    auto it = m_players.constFind(playerId);

    if (it == m_players.cend()) {
        throw std::invalid_argument("Invalid ID of a player");
    }

    // Referenca na postojecu sesiju, bez kopiranja.
    const PlayerNetworkSession& session = it.value();

    if (msg.type() == MessageType::PositionSnapshot) {
        trySendWithUdp(msg, msg.type(), session);
        return;
    }

    trySendWithTCP(msg, msg.type(), session);
}
void Server::kick(int playerId)
{
    auto it = m_players.find(playerId);

    if (it == m_players.end()) {
        return;
    }

    PlayerNetworkSession session = it.value();

    m_players.erase(it);
    m_lastMoveTick.remove(playerId);

    if (session.tcpConnection != nullptr) {
        session.tcpConnection->deleteLater();
    }

    emit playerDisconnected(playerId);
}

void Server::onNewConnection()
{
    while (m_tcp.hasPendingConnections()) {
        QTcpSocket* playerSocket = m_tcp.nextPendingConnection();

        if (playerSocket == nullptr) {
            continue;
        }

        int playerId = m_nextPlayerId++;

        ClientConnection* connection = new ClientConnection(
            playerSocket,
            playerId,
            this
            );

        // ClientConnection postaje vlasnik TCP socket-a.
        playerSocket->setParent(connection);

        PlayerNetworkSession session;
        session.tcpConnection = connection;
        session.udpToken =
            QUuid::createUuid().toString(QUuid::WithoutBraces);

        m_players.insert(playerId, session);

        connect(
            connection,
            &ClientConnection::messageReceived,
            this,
            &Server::messageReceived
            );

        connect(
            connection,
            &ClientConnection::disconnected,
            this,
            &Server::onPlayerDisconnected
            );

        emit playerConnected(playerId);
    }
}

void Server::onUdpReadyRead()
{
    constexpr qint64 maxDatagramSize = 1200;

    while (m_udp.hasPendingDatagrams()) {
        qint64 datagramSize = m_udp.pendingDatagramSize();

        if (datagramSize < 0) {
            return;
        }

        // Odbacujemo prazan ili prevelik datagram.
        if (datagramSize == 0 || datagramSize > maxDatagramSize) {
            m_udp.readDatagram(nullptr, 0);
            continue;
        }

        QByteArray datagram;
        datagram.resize(static_cast<int>(datagramSize));

        QHostAddress senderAddress;
        quint16 senderPort = 0;

        qint64 bytesRead = m_udp.readDatagram(datagram.data(), datagram.size(), &senderAddress, &senderPort);

        if (bytesRead != datagramSize) {
            continue;
        }

        // Parsiramo JSON jednom.
        QJsonParseError error;
        QJsonDocument document = QJsonDocument::fromJson(datagram, &error);

        if (error.error != QJsonParseError::NoError || !document.isObject()) {
            continue;
        }

        QJsonObject jsonObject = document.object();
        int rawType = jsonObject.value("type").toInt(-1);

        // Server preko UDP-a prima samo registraciju i kretanje.
        if (rawType != static_cast<int>(MessageType::UdpRegister) && rawType != static_cast<int>(MessageType::MoveInput)) {
            continue;
        }

        if (!jsonObject.value("messageContent").isObject()) {
            continue;
        }

        MessageType type = static_cast<MessageType>(rawType);
        QJsonObject content = jsonObject.value("messageContent").toObject();

        // Pronalazimo igraca kome poruka pripada.
        int playerId = content.value("playerId").toInt(-1);
        auto it = m_players.find(playerId);

        if (it == m_players.end()) {
            continue;
        }

        PlayerNetworkSession& session = it.value();
        QString token = content.value("udpToken").toString();

        if (token.isEmpty() || token != session.udpToken) {
            continue;
        }

        // Registrujemo adresu i port sa kojih je datagram stigao.
        if (type == MessageType::UdpRegister) {
            session.udpAddress = senderAddress;
            session.udpPort = senderPort;

            Message reply(MessageType::UdpRegistered, {{"playerId", playerId}});
            m_udp.writeDatagram(reply.toBytes(), senderAddress, senderPort);

            // Ponavljanje registracije ne resetuje poslednji tick.
            continue;
        }

        // Preostali tip je MoveInput.
        if (session.udpPort == 0 || session.udpAddress.isNull() || session.udpAddress != senderAddress || session.udpPort != senderPort) {
            continue;
        }

        // Poruka mora pripadati trenutnom periodu igre.
        int phaseId = content.value("phaseId").toInt(-1);

        if (phaseId < 0 || static_cast<quint32>(phaseId) != m_phaseId) {
            continue;
        }

        // Tick mora biti pozitivan ceo broj koji staje u quint32.
        double tickValue = content.value("tick").toDouble(-1);

        if (!std::isfinite(tickValue) || tickValue < 1 || tickValue > std::numeric_limits<quint32>::max() || std::floor(tickValue) != tickValue) {
            continue;
        }

        quint32 tick = static_cast<quint32>(tickValue);

        // Odbacujemo duplirane i zastarele poruke.
        if (m_lastMoveTick.contains(playerId) && tick <= m_lastMoveTick.value(playerId)) {
            continue;
        }

        m_lastMoveTick.insert(playerId, tick);

        // Pravimo Message iz vec procitanih podataka.
        Message message(type, content);
        emit messageReceived(playerId, message);
    }
}

void Server::onPlayerDisconnected(int playerId)
{
    auto it = m_players.find(playerId);

    if (it == m_players.end()) {
        return;
    }

    PlayerNetworkSession session = it.value();

    m_players.erase(it);
    m_lastMoveTick.remove(playerId);

    if (session.tcpConnection != nullptr) {
        session.tcpConnection->deleteLater();
    }

    emit playerDisconnected(playerId);
}