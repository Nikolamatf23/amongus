#include "network/Message.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

Message::Message(MessageType type,
                 const QJsonObject& messageContent)
    : m_type(type),
    m_messageContent(messageContent)
{
}

MessageType Message::type() const
{
    return m_type;
}

QJsonObject& Message::get_messageContent()
{
    return m_messageContent;
}

QByteArray Message::toBytes() const
{
    QJsonObject jsonObject;

    jsonObject["type"] =
        static_cast<int>(m_type);

    jsonObject["messageContent"] =
        m_messageContent;

    QJsonDocument document(jsonObject);

    return document.toJson(
        QJsonDocument::Compact
        );
}

Message Message::fromBytes(const QByteArray& bytes)
{
    QJsonParseError error;

    QJsonDocument document =
        QJsonDocument::fromJson(bytes, &error);

    if (error.error != QJsonParseError::NoError ||
        !document.isObject()) {
        return Message(MessageType::Invalid);
        }

    QJsonObject jsonObject =
        document.object();

    // Ako polja "type" nema ili nije ceo broj, toInt(-1) vraca -1.
    // Bez rezervne vrednosti -1 dobili bismo 0, a to je Join.
    const int rawType = jsonObject.value("type").toInt(-1);

    // Invalid je poslednji u enumu, pa je njegova vrednost
    // ujedno broj svih pravih tipova.
    if (rawType < 0 ||
        rawType >= static_cast<int>(MessageType::Invalid)) {
        return Message(MessageType::Invalid);
        }

    MessageType type = static_cast<MessageType>(rawType);

    QJsonObject messageContent =
        jsonObject["messageContent"].toObject();

    return Message(type, messageContent);
}