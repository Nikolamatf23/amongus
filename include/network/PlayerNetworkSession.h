#ifndef PLAYERNETWORKSESSION_H
#define PLAYERNETWORKSESSION_H

#include <QHostAddress>
#include <QString>
#include <QtGlobal>

class ClientConnection;

struct PlayerNetworkSession
{
    ClientConnection* tcpConnection = nullptr;
    QHostAddress udpAddress;
    quint16 udpPort = 0;
    //Bez tokena bi neko mogao samo da napiše "playerId": 3 i pokuša da
    //šalje kretanje kao igrač 3. Token je tajna oznaka sesije; ne šifruje poruku.
    QString udpToken;
    quint32 last_move_tick;
};

#endif // PLAYERNETWORKSESSION_H