#ifndef MESSAGE_H
#define MESSAGE_H

#include <QByteArray>
#include <QJsonObject>

#include "MessageType.h"

class Message
{
public:
    // KONSTRUKTOR: pravi poruku sa zadatim tipom i sadrzajem.
    explicit Message(
        MessageType type,
        const QJsonObject& messageContent = QJsonObject()
        );

    // METODA: vraca tip poruke.
    MessageType type() const;

    // METODA: vraca referencu na JSON sadrzaj poruke. (getter)
    QJsonObject& get_messageContent();

    // METODA: pretvara poruku u bajtove za slanje.
    QByteArray toBytes() const;

    // STATICKA METODA: pravi poruku iz primljenog JSON-a.
    static Message fromBytes(const QByteArray& bytes);

private:
    // POLJE: cuva tip poruke.
    MessageType m_type;

    // POLJE: cuva JSON sadrzaj poruke.
    QJsonObject m_messageContent;
};

#endif // MESSAGE_H