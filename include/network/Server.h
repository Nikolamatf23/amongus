#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QUdpSocket>
#include <QMap>

#include "network/Message.h"
#include "network/PlayerNetworkSession.h"

class Server : public QObject
{
    Q_OBJECT

public:
    explicit Server(QObject* parent = nullptr);

    bool startListening(
        quint16 tcpPort,
        quint16 udpPort
        );

    void stop();

    void broadcast(const Message& msg);
    void sendTo(int playerId, const Message& msg);
    void kick(int playerId);
    bool trySendWithUdp(const Message& msg, MessageType type, const PlayerNetworkSession& playerNetworkSession);
    bool trySendWithTCP(const Message& msg, MessageType type, const PlayerNetworkSession& playerNetworkSession);


signals:
    void messageReceived(int playerId, const Message& msg);
    void playerConnected(int playerId);
    void playerDisconnected(int playerId);

private slots:
    void onNewConnection();
    void onUdpReadyRead();
    void onPlayerDisconnected(int playerId);

private:
    QTcpServer m_tcp;
    QUdpSocket m_udp;

    // Jedan zapis po igracu: TCP veza + UDP adresa , port i tick.
    QMap<int, PlayerNetworkSession> m_players;

    int m_nextPlayerId = 1;
    //phaseId služi da server zna iz kog perioda igre dolazi UDP poruka.
    //da ako je neki deo pre tri sastanka, da znamo da njega ne uzimamo
    quint32 m_phaseId = 0;
};

#endif // SERVER_H