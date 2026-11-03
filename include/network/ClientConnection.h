#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QHostAddress>

#include "network/Message.h"

class ClientConnection : public QObject
{
    // Omogucava Qt signale i slotove.
    Q_OBJECT

public:
    // KONSTRUKTOR: prima vec povezani socket od Server klase
    // i ID igraca kome ta konekcija pripada.
    explicit ClientConnection(
        QTcpSocket* playerSocket,
        int playerId,
        QObject* parent = nullptr
        );

    // GETTER: vraca ID igraca ove konekcije.
    int playerId() const;

    // METODA: pretvara Message u bajtove i salje ih ovom igracu.
    void send(const Message& message);

    // GETTER: vraca IP adresu povezanog klijenta.
    QHostAddress peerAddress() const;

signals:
    // SIGNAL: obavestava Server da je stigla kompletna poruka.
    // ID se uzima iz ove konekcije.
    void messageReceived(int playerId, const Message& message);

    // SIGNAL: obavestava Server da je ovaj igrac prekinuo vezu.
    void disconnected(int playerId);

private slots:
    // SLOT: cita pristigle bajtove, dopunjuje buffer
    // i izdvaja sve kompletne poruke.
    void onReadyRead();

    // SLOT: aktivira se kada se socket diskonektuje.
    // Prosledjuje Server-u ID igraca koji je prekinuo vezu.
    void onDisconnected();

private:
    // POLJE: socket za komunikaciju sa jednim klijentom.
    QTcpSocket* m_playerSocket;

    // POLJE: cuva bajtove dok ne stigne kompletna poruka.
    QByteArray m_buffer;

    // POLJE: ID igraca koji je Server dodelio ovoj konekciji.
    int m_playerId;
};

#endif // CLIENTCONNECTION_H