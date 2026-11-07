#include "game/Message.h"

#include <QJsonDocument>

// Pamti tip i JSON sadržaj nove poruke.
Message::Message(MessageType type, const QJsonObject& messageContent)
    : m_type(type), m_messageContent(messageContent)
{
}

// Vraća vrstu poruke, npr. MoveInput ili Vote.
MessageType Message::type() const
{
    return m_type;
}

// Referenca omogućava pozivaocu da pročita ili dopuni sadržaj poruke.
QJsonObject& Message::get_messageContent()
{
    return m_messageContent;
}

// Priprema JSON bajtove. Mrežni sloj dodaje prefiks dužine za TCP slanje.
QByteArray Message::toBytes() const
{
    // Tip se šalje kao broj, a sadržaj kao ugnježdeni JSON objekat.
    const QJsonObject object{
        {"type", static_cast<int>(m_type)},
        {"messageContent", m_messageContent}
    };
    // Compact izostavlja razmake i nove redove radi manje poruke.
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

// Čita JSON deo primljene poruke i proverava osnovnu strukturu.
Message Message::fromBytes(const QByteArray& bytes)
{
    const QJsonDocument document = QJsonDocument::fromJson(bytes);
    // Neispravan JSON i vrednosti koje nisu objekti postaju Invalid poruka.
    if (!document.isObject()) {
        return Message(MessageType::Invalid);
    }

    const QJsonObject object = document.object();
    const int type = object.value("type").toInt(-1); // -1 označava nedostajući ili neispravan broj tipa.
    const QJsonValue content = object.value("messageContent");
    // Prihvatamo poznate tipove poruka i sadržaj predstavljen JSON objektom.
    if (type < static_cast<int>(MessageType::Join) ||
        type >= static_cast<int>(MessageType::Invalid) || !content.isObject()) {
        return Message(MessageType::Invalid);
    }

    // Validiran broj pretvaramo u enum i pravimo poruku sa dobijenim sadržajem.
    return Message(static_cast<MessageType>(type), content.toObject());
}
