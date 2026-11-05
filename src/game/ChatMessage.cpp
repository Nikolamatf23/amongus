#include "game/ChatMessage.h"

ChatMessage::ChatMessage(int senderId,
                         const QString& senderName,
                         const QString& text,
                         const QDateTime& time)
    : m_senderId(senderId)
    , m_senderName(senderName)
    , m_text(text.trimmed().left(200))   // bez razmaka na krajevima, max 200 znakova
    , m_time(time)
{
}

QJsonObject ChatMessage::toJson() const
{
    QJsonObject obj;
    obj["senderId"]   = m_senderId;
    obj["senderName"] = m_senderName;
    obj["text"]       = m_text;
    // Vreme kao broj milisekundi od 1970 - lako za prenos.
    obj["time"]       = static_cast<double>(m_time.toMSecsSinceEpoch());
    return obj;
}

ChatMessage ChatMessage::fromJson(const QJsonObject& obj)
{
    const qint64 ms = static_cast<qint64>(obj.value("time").toDouble());

    return ChatMessage(obj.value("senderId").toInt(-1),
                       obj.value("senderName").toString(),
                       obj.value("text").toString(),
                       QDateTime::fromMSecsSinceEpoch(ms));
}
