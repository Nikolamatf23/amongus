#ifndef GAME_MESSAGE_H
#define GAME_MESSAGE_H

#include "MessageTypes.h"

#include <QByteArray>
#include <QJsonObject>

class Message
{
public:
    // Pravi poruku sa zadatim tipom i opcionim JSON sadržajem.
    explicit Message(MessageType type, const QJsonObject& messageContent = {});

    MessageType type() const;          // Vraća tip poruke.
    QJsonObject& get_messageContent(); // Daje pristup sadržaju za čitanje i izmenu.
    QByteArray toBytes() const;        // Pretvara poruku u JSON bajtove.

    // Pravi poruku iz primljenih JSON bajtova.
    static Message fromBytes(const QByteArray& bytes);

private:
    MessageType m_type;           // Vrsta poruke, npr. MoveInput.
    QJsonObject m_messageContent; // Podaci poruke, npr. dx i dy za kretanje.
};

#endif // GAME_MESSAGE_H
