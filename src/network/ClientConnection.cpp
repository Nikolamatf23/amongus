#include "network/ClientConnection.h"

#include <QDataStream>
#include <QIODevice>

ClientConnection::ClientConnection(
    QTcpSocket* playerSocket,
    int playerId,
    QObject* parent
    )
    : QObject(parent),
    m_playerSocket(playerSocket),
    m_playerId(playerId)
{
    //znaci kada stigne poruke preko socket-a automatski se pali
    //readyRead koji ce automatski da opali onReadyRead
    // koji ce dalje da procita pristigle podatke doda ih u bafer
    // proveri da li je stigla cela poruka
    // uradi fromBytes
    //i emituje-aktivira signal na serveru
    connect(m_playerSocket,
            &QTcpSocket::readyRead,
            this,
            &ClientConnection::onReadyRead);

    //&QTcpSocket::disconnected
    //Signal koji pratimo. Qt ga emituje kada se socket prekine.
    connect(m_playerSocket,
            &QTcpSocket::disconnected,
            this,
            &ClientConnection::onDisconnected);
}

int ClientConnection::playerId() const
{
    return m_playerId;
}

QHostAddress ClientConnection::peerAddress() const
{
    return m_playerSocket->peerAddress();
}

void ClientConnection::send(const Message& message)
{
    QByteArray messageBytes = message.toBytes();

    // Pravimo zaglavlje koje čuva veličinu JSON poruke
    QByteArray header;

    //pravimo stream preko kojeg cemo moci da upisem
    //velicinu same poruke u header
    QDataStream stream(
        &header,
        QIODevice::WriteOnly
        );

    stream.setByteOrder(
        QDataStream::BigEndian
        );

    stream << quint32(messageBytes.size());

    // Paket: [veličina poruke][JSON bajtovi]
    QByteArray packet = header + messageBytes;

    m_playerSocket->write(packet);
}

void ClientConnection::onReadyRead()
{
    // Dodajemo nove bajtove u postojeći bafer
    // ono sto je stiglo
    m_buffer.append(m_playerSocket->readAll());

    // Jedan TCP paket ne mora sadržati celu poruku,
    // zato proveravamo da li je stigla kompletna poruka
    while (true) {
        //Ova provera proverava da li su stigla prva 4 bajta zaglavlja.
        if (m_buffer.size() < 4) {
            return;
        }

        // uzimamo prva cetiri bajta iz bafera, to je zaglavlje
        // koje sadrzi velicinu json poruke
        QByteArray header = m_buffer.left(4);

        // pravimo stream za citanja za ta 4 bajta
        QDataStream stream(
            &header,
            QIODevice::ReadOnly
            );

        //kaze da je broj zapisan u BigEndian formatu
        //BigEndian određuje redosled bajtova
        //kada se broj čuva u memoriji ili šalje preko mreže.
        // to je stvar dogovora
        //Ako klijent šalje veličinu u BigEndian formatu,
        //server mora tako i da je čita. Oba kraja veze moraju koristiti isti redosled bajtova.
        stream.setByteOrder(
            QDataStream::BigEndian
            );
        //nije obican int jer OBICNO bude 4 bajta
        // quint32 - on garantuje bas 4 bajta
        quint32 messageSize = 0;

        stream >> messageSize;
        // 4 - cetiri bajta koja govore o velicini poruke
        // messageSize - broj bajtova ostatka poruke
        if (m_buffer.size() < 4 + messageSize) {
            // ovde cemo izaci iz while-a ali necemo prazniti buffer
            // vec ce ostati podaci koji su do sada stigli
            // i tek kada u njega upisemo celu poruku tek cemo onda
            // poslati dalje
            return;
        }

        QByteArray messageBytes = m_buffer.mid(4, messageSize);

        m_buffer.remove(0, 4 + messageSize);


        Message message = Message::fromBytes(messageBytes);

        if (message.type() == MessageType::Invalid) {
            continue;
        }
        // AKO NIKAD NE STIGNE CELA PORUKA
        // BAFER CE OSTATI DONEKLE PUN I ZIVETI SVE DOK ZIVI
        // OBJEKAT CLIENTCONNECTION ALI POSTO GA PRAVIMO DINAMICKI
        // NIJE STRASNO, MOZEMO OSTAVITI OVAKO
        // jer ce se buffer oslobodiiti kada uradimo delete connection na serveru

        emit messageReceived(
            m_playerId,
            message
            );
    }
}

void ClientConnection::onDisconnected()
{
    // Obavestavamo Server koji igrac je prekinuo vezu.
    emit disconnected(m_playerId);
}