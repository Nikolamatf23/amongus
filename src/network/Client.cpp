#include "network/Client.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QtEndian>

namespace {
constexpr int kMaxDatagramSize = 1200;           // isto kao na serveru
constexpr qint64 kMaxTcpMessageSize = 4 * 1024 * 1024;
constexpr int kUdpRegisterIntervalMs = 300;
constexpr int kMaxRegisterAttempts = 20;         // ~6 s
}

Client::Client(QObject* parent)
    : QObject(parent)
{
    connect(&m_socket, &QTcpSocket::connected,
            this, &Client::onTcpConnected);
    connect(&m_socket, &QTcpSocket::disconnected,
            this, &Client::onTcpDisconnected);
    connect(&m_socket, &QTcpSocket::readyRead,
            this, &Client::onReadyRead);
    connect(&m_socket, &QAbstractSocket::errorOccurred,
            this, &Client::onSocketError);

    connect(&m_udp, &QUdpSocket::readyRead,
            this, &Client::onUdpReadyRead);

    m_udpRegisterTimer.setInterval(kUdpRegisterIntervalMs);
    connect(&m_udpRegisterTimer, &QTimer::timeout,
            this, &Client::registerUdp);
}

void Client::connectToHost(const QString& host, quint16 tcpPort)
{
    resetSession();
    m_socket.connectToHost(host, tcpPort);
}

void Client::disconnectFromHost()
{
    m_udpRegisterTimer.stop();
    m_udp.close();
    m_socket.disconnectFromHost();
}

bool Client::isConnected() const
{
    return m_socket.state() == QAbstractSocket::ConnectedState;
}

int Client::myId() const
{
    return m_myId;
}

bool Client::isUdpReady() const
{
    return m_udpReady;
}

void Client::resetSession()
{
    m_udpRegisterTimer.stop();
    m_udp.close();
    m_buffer.clear();
    m_myId = -1;
    m_udpToken.clear();
    m_udpReady = false;
    m_serverUdpPort = 0;
    m_moveTick = 0;
    m_lastPositionTick = 0;
    m_phaseId = 0;
    m_registerAttempts = 0;
}

// ---------------------------------------------------------------- slanje

void Client::send(const Message& msg)
{
    switch (msg.type()) {
    case MessageType::MoveInput:
        sendUdp(msg);
        break;

    // Ove poruke Client sam salje/prima, kontroler ih ne sme slati.
    case MessageType::UdpRegister:
    case MessageType::UdpRegistered:
    case MessageType::PositionSnapshot:
        break;

    default:
        sendTcp(msg);
        break;
    }
}

void Client::sendTcp(const Message& msg)
{
    if (!isConnected()) {
        return;
    }

    const QByteArray body = msg.toBytes();

    QByteArray header(4, 0);
    qToBigEndian<quint32>(static_cast<quint32>(body.size()),
                          reinterpret_cast<uchar*>(header.data()));

    m_socket.write(header + body);
}

void Client::sendUdp(const Message& msg)
{
    // Dok UDP nije registrovan, kretanje se odbacuje.
    // Nista ne fali: sledeci input stize za nekoliko desetina ms.
    if (!m_udpReady) {
        return;
    }

    Message outgoing = msg;
    QJsonObject& content = outgoing.get_messageContent();

    content["playerId"] = m_myId;
    content["udpToken"] = m_udpToken;
    content["phaseId"] = static_cast<int>(m_phaseId);
    content["tick"] = static_cast<double>(++m_moveTick);   // server trazi >= 1

    const QByteArray datagram = outgoing.toBytes();

    if (datagram.size() > kMaxDatagramSize) {
        return;
    }

    m_udp.writeDatagram(datagram, m_serverAddress, m_serverUdpPort);
}

// ---------------------------------------------------------------- TCP

void Client::onTcpConnected()
{
    m_serverAddress = m_socket.peerAddress();
    emit connected();
}

void Client::onTcpDisconnected()
{
    m_udpRegisterTimer.stop();
    m_udp.close();
    m_udpReady = false;
    emit disconnected();
}

void Client::onSocketError()
{
    emit errorOccurred(m_socket.errorString());
}

void Client::onReadyRead()
{
    m_buffer.append(m_socket.readAll());

    while (true) {
        if (m_buffer.size() < 4) {
            return;
        }

        const quint32 messageSize = qFromBigEndian<quint32>(
            reinterpret_cast<const uchar*>(m_buffer.constData()));

        if (messageSize > kMaxTcpMessageSize) {
            emit errorOccurred("Server je poslao neispravnu poruku.");
            m_buffer.clear();
            m_socket.abort();
            return;
        }

        if (m_buffer.size() < 4 + static_cast<qint64>(messageSize)) {
            return;   // cela poruka jos nije stigla
        }

        const QByteArray messageBytes = m_buffer.mid(4, messageSize);
        m_buffer.remove(0, 4 + messageSize);

        // Message::fromBytes pri gresci vraca Invalid - takve poruke odbacujemo.
        Message message = Message::fromBytes(messageBytes);
        if (message.type() == MessageType::Invalid) {
            continue;
        }

        handleIncoming(message);
    }
}

// ---------------------------------------------------------------- UDP

void Client::registerUdp()
{
    if (m_udpReady) {
        m_udpRegisterTimer.stop();
        return;
    }

    if (++m_registerAttempts > kMaxRegisterAttempts) {
        m_udpRegisterTimer.stop();
        emit errorOccurred("UDP registracija nije uspela.");
        return;
    }

    Message reg(MessageType::UdpRegister,
                {{"playerId", m_myId},
                 {"udpToken", m_udpToken}});

    m_udp.writeDatagram(reg.toBytes(), m_serverAddress, m_serverUdpPort);
}

void Client::onUdpReadyRead()
{
    while (m_udp.hasPendingDatagrams()) {
        const qint64 size = m_udp.pendingDatagramSize();

        if (size <= 0 || size > kMaxDatagramSize) {
            m_udp.readDatagram(nullptr, 0);   // odbaci
            continue;
        }

        QByteArray datagram;
        datagram.resize(static_cast<int>(size));

        QHostAddress senderAddress;
        quint16 senderPort = 0;

        if (m_udp.readDatagram(datagram.data(), datagram.size(),
                               &senderAddress, &senderPort) != size) {
            continue;
        }

        // Prihvatamo samo datagrame sa servera.
        if (senderPort != m_serverUdpPort ||
            !senderAddress.isEqual(m_serverAddress,
                                   QHostAddress::TolerantConversion)) {
            continue;
        }

        QJsonParseError error;
        const QJsonDocument doc = QJsonDocument::fromJson(datagram, &error);
        if (error.error != QJsonParseError::NoError || !doc.isObject()) {
            continue;
        }

        Message message = Message::fromBytes(datagram);

        if (message.type() == MessageType::UdpRegistered) {
            m_udpReady = true;
            m_udpRegisterTimer.stop();
            continue;
        }

        if (message.type() != MessageType::PositionSnapshot) {
            continue;
        }

        // Zastita od starih podataka: iz stare faze ili stariji tick.
        const QJsonObject& content = message.get_messageContent();

        const int phaseId = content.value("phaseId").toInt(-1);
        if (phaseId < 0 || static_cast<quint32>(phaseId) != m_phaseId) {
            continue;
        }

        const double tickValue = content.value("tick").toDouble(-1);
        if (tickValue < 1) {
            continue;
        }

        const quint32 tick = static_cast<quint32>(tickValue);
        if (tick <= m_lastPositionTick) {
            continue;
        }
        m_lastPositionTick = tick;

        handleIncoming(message);
    }
}

// ---------------------------------------------------------------- zajednicko

void Client::handleIncoming(const Message& msg)
{
    Message incoming = msg;
    const QJsonObject content = incoming.get_messageContent();

    switch (msg.type()) {
    case MessageType::JoinAccepted:
        m_myId = content.value("playerId").toInt(-1);
        m_udpToken = content.value("udpToken").toString();
        m_serverUdpPort =
            static_cast<quint16>(content.value("serverUdpPort").toInt(0));

        if (m_myId >= 0 && !m_udpToken.isEmpty() && m_serverUdpPort != 0) {
            if (m_udp.state() != QAbstractSocket::BoundState &&
                !m_udp.bind(QHostAddress::Any, 0)) {
                emit errorOccurred("Ne mogu da otvorim UDP port.");
            } else {
                m_registerAttempts = 0;
                m_udpReady = false;
                registerUdp();
                m_udpRegisterTimer.start();
            }
        }
        break;

    case MessageType::StartGame:
        // Server tada vraca phaseId na 0 i brise poslednje tikove.
        m_phaseId = 0;
        m_lastPositionTick = 0;
        break;

    case MessageType::PhaseChanged:
        m_phaseId = static_cast<quint32>(content.value("phaseId").toInt(0));
        m_lastPositionTick = 0;
        break;

    default:
        break;
    }

    emit messageReceived(msg);
}