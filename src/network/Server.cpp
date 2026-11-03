#include "network/Server.h"
#include "network/ClientConnection.h"

#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QTcpSocket>
#include <QUuid>

#include <cmath>
#include <limits>

Server::Server(QObject* parent)
    : QObject(parent)
{
    connect(
        &m_tcp,
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

void Server::broadcast(const Message& msg)
{
    Message outgoing = msg;

    if (msg.type() == MessageType::StartGame) {
        m_phaseId = 0;
        m_lastMoveTick.clear();
    }

    if (msg.type() == MessageType::PhaseChanged) {
        ++m_phaseId;
        m_lastMoveTick.clear();

        // Klijenti tako saznaju koji phaseId treba da stave
        // u naredne UDP poruke za kretanje.
        outgoing.get_messageContent()["phaseId"] =
            static_cast<int>(m_phaseId);
    }

    if (msg.type() == MessageType::PositionSnapshot) {
        QByteArray datagram = outgoing.toBytes();

        // Jedan UDP datagram treba da bude mali.
        // GameController deli pozicije u vise poruka ako je potrebno.
        if (datagram.size() > 1200) {
            return;
        }

        for (const PlayerNetworkSession& session : m_players) {
            if (session.udpPort == 0 ||
                session.udpAddress.isNull()) {
                continue;
            }

            m_udp.writeDatagram(
                datagram,
                session.udpAddress,
                session.udpPort
                );
        }

        return;
    }

    // Svi ostali događaji koje braadcast šalje
    // idu kroz postojeće TCP veze.
    for (const PlayerNetworkSession& session : m_players) {
        if (session.tcpConnection != nullptr) {
            session.tcpConnection->send(outgoing);
        }
    }
}

// GameController poziva ovo kada salje jednom igracu.
// const playerId jer necemo da ga menjamo
void Server::sendTo(const int playerId, const Message& msg)
{
    auto it = m_players.find(playerId);

    if (it == m_players.end()) {
        return;
    }

    PlayerNetworkSession& session = it.value();
    Message outgoing = msg;

    if (msg.type() == MessageType::PositionSnapshot) {
        if (session.udpPort == 0 ||
            session.udpAddress.isNull()) {
            return;
        }

        QByteArray datagram = outgoing.toBytes();

        if (datagram.size() > 1200) {
            return;
        }

        m_udp.writeDatagram(
            datagram,
            session.udpAddress,
            session.udpPort
            );

        return;
    }

    if (msg.type() == MessageType::JoinAccepted) {
        // Postojeci JoinAccepted obogacujemo mreznim podacima.
        // GameController i dalje koristi sendTo kao ranije.
        QJsonObject& content = outgoing.get_messageContent();

        content["playerId"] = playerId;
        content["udpToken"] = session.udpToken;
        content["serverUdpPort"] =
            static_cast<int>(m_udp.localPort());
    }

    if (msg.type() == MessageType::PhaseChanged) {
        outgoing.get_messageContent()["phaseId"] =
            static_cast<int>(m_phaseId);
    }

    if (session.tcpConnection != nullptr) {
        session.tcpConnection->send(outgoing);
    }
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
    while (m_udp.hasPendingDatagrams()) {
        qint64 datagramSize =
            m_udp.pendingDatagramSize();

        if (datagramSize < 0) {
            return;
        }

        // Odbaci prazan ili prevelik datagram.
        if (datagramSize == 0 ||
            datagramSize > 1200) {

            m_udp.readDatagram(nullptr, 0);
            continue;
        }

        QByteArray datagram;
        datagram.resize(
            static_cast<int>(datagramSize)
        );

        QHostAddress senderAddress;
        quint16 senderPort = 0;

        qint64 bytesRead =
            m_udp.readDatagram(
                datagram.data(),
                datagram.size(),
                &senderAddress,
                &senderPort
            );

        if (bytesRead != datagramSize) {
            continue;
        }

        // Prvo proveravamo da li je datagram validan JSON.
        QJsonParseError error;

        QJsonDocument document =
            QJsonDocument::fromJson(
                datagram,
                &error
            );

        if (error.error != QJsonParseError::NoError ||
            !document.isObject()) {
            continue;
        }

        QJsonObject jsonObject =
            document.object();

        // Proveravamo da li JSON ima type.
        int rawType =
            jsonObject.value("type").toInt(-1);

        // Preko UDP-a server prihvata samo ove tipove.
        if (rawType !=
                static_cast<int>(
                    MessageType::UdpRegister
                ) &&
            rawType !=
                static_cast<int>(
                    MessageType::MoveInput
                )) {
            continue;
        }

        // messageContent mora biti JSON objekat.
        if (!jsonObject.value("messageContent")
                 .isObject()) {
            continue;
        }

        // Pretvaramo bajtove u našu Message klasu.
        Message message =
            Message::fromBytes(datagram);

        // Ako pretvaranje nije uspelo, odbaci poruku.
        if (message.type() ==
            MessageType::Invalid) {
            continue;
        }

        QJsonObject content =
            message.get_messageContent();

        // Čitamo ID igrača.
        int playerId =
            content.value("playerId").toInt(-1);

        auto it =
            m_players.find(playerId);

        // Igrač ne postoji u mapi.
        if (it == m_players.end()) {
            continue;
        }

        PlayerNetworkSession& session =
            it.value();

        // Čitamo token iz poruke.
        QString token =
            content.value("udpToken").toString();

        // Token mora biti ispravan.
        if (token.isEmpty() ||
            token != session.udpToken) {
            continue;
        }

        // UDP registracija.
        if (message.type() ==
            MessageType::UdpRegister) {

            // Adresu i port uzimamo iz stvarnog
            // pristiglog datagrama.
            session.udpAddress =
                senderAddress;

            session.udpPort =
                senderPort;

            // Novi UDP endpoint počinje novi
            // redosled MoveInput poruka.
            m_lastMoveTick.remove(playerId);

            Message reply(
                MessageType::UdpRegistered,
                {
                    {"playerId", playerId}
                }
            );

            // Potvrdu šaljemo na adresu i port
            // sa kojih je stigao UdpRegister.
            m_udp.writeDatagram(
                reply.toBytes(),
                senderAddress,
                senderPort
            );

            continue;
        }

        // MoveInput prihvatamo samo sa adrese
        // i porta koji su registrovani.
        if (session.udpPort == 0 ||
            session.udpAddress.isNull() ||
            session.udpAddress != senderAddress ||
            session.udpPort != senderPort) {
            continue;
        }

        // Čitamo fazu igre.
        int phaseId =
            content.value("phaseId").toInt(-1);

        // Poruka mora pripadati trenutnoj fazi.
        if (phaseId < 0 ||
            static_cast<quint32>(phaseId) !=
                m_phaseId) {
            continue;
        }

        // Tick može doći iz JSON-a kao double,
        // zato prvo proveravamo da li je ceo broj.
        double tickValue =
            content.value("tick").toDouble(-1);

        if (tickValue < 1 ||
            tickValue >
                std::numeric_limits<quint32>::max() ||
            std::floor(tickValue) != tickValue) {
            continue;
        }

        quint32 tick =
            static_cast<quint32>(tickValue);

        // Odbacujemo staru ili dupliranu poruku.
        if (m_lastMoveTick.contains(playerId) &&
            tick <=
                m_lastMoveTick.value(playerId)) {
            continue;
        }

        // Pamtimo poslednji prihvaćeni tick.
        m_lastMoveTick.insert(
            playerId,
            tick
        );

        // GameController dobija validan MoveInput.
        emit messageReceived(
            playerId,
            message
        );
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