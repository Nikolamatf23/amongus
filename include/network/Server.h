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

    // Jedan zapis po igracu: TCP veza + UDP adresa i port.
    QMap<int, PlayerNetworkSession> m_players;

    int m_nextPlayerId = 1;

    // Poslednji prihvaceni UDP tick za svakog igraca.
    //tick nam sluzi da damo redosled porukama i npr ako idu poruke sa sledecim tick-omvima
    // 1-> 2 -> 3 -> 5 i sad dodje neki 4 njega necemo uzeti nego ga odbacujemo jer zastareo i idemo dalje
    QMap<int, quint32> m_lastMoveTick;
    //phaseId služi da server zna iz kog perioda igre dolazi UDP poruka.
    quint32 m_phaseId = 0;
};

#endif // SERVER_H