#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QHostAddress>
#include <QByteArray>
#include <QString>
#include <QTimer>

#include "network/Message.h"

// Klijentski kraj mreznog sloja.
// TCP: sve pouzdane poruke (Join, Kill, Vote, Chat...), okvir [4B duzine][JSON].
// UDP: samo MoveInput (ka serveru) i PositionSnapshot (od servera).
class Client : public QObject
{
    Q_OBJECT

public:
    explicit Client(QObject* parent = nullptr);

    // Prvo se otvara TCP; UDP se registruje tek kad stigne JoinAccepted.
    void connectToHost(const QString& host, quint16 tcpPort);
    void disconnectFromHost();

    // Sam bira transport po tipu poruke. Kod MoveInput-a sam dodaje
    // playerId, udpToken, phaseId i tick - kontroler to ne mora da zna.
    void send(const Message& msg);

    bool isConnected() const;   // stanje TCP veze
    int myId() const;           // -1 dok ne stigne JoinAccepted
    bool isUdpReady() const;

signals:
    void connected();
    void disconnected();
    void messageReceived(const Message& msg);
    void errorOccurred(const QString& text);

private slots:
    void onTcpConnected();
    void onTcpDisconnected();
    void onReadyRead();        // TCP okvirovanje
    void onUdpReadyRead();     // UDP: provera posiljaoca, phaseId, tick
    void registerUdp();        // ponavlja se dok server ne odgovori
    void onSocketError();

private:
    void sendTcp(const Message& msg);
    void sendUdp(const Message& msg);
    void handleIncoming(const Message& msg);   // zajednicko za TCP i UDP
    void resetSession();

    QTcpSocket m_socket;
    QByteArray m_buffer;                        // samo za TCP

    QUdpSocket m_udp;
    QHostAddress m_serverAddress;
    quint16 m_serverUdpPort = 0;

    int m_myId = -1;
    QString m_udpToken;
    bool m_udpReady = false;

    quint32 m_moveTick = 0;
    quint32 m_lastPositionTick = 0;
    quint32 m_phaseId = 0;

    QTimer m_udpRegisterTimer;
    int m_registerAttempts = 0;
};

#endif // CLIENT_H
