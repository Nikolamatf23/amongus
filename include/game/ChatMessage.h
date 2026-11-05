#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

// ============================================================================
//  ChatMessage
//  Jedna poruka u chatu tokom sastanka. Cuva je VotingSession (m_chat).
//
//  Dijagram:
//    - m_senderId: int        - m_senderName: QString
//    - m_text: QString        - m_time: QDateTime
//    + toJson(): QJsonObject
//    + static fromJson(obj): ChatMessage
// ============================================================================

#include <QDateTime>
#include <QJsonObject>
#include <QMetaType>
#include <QString>

class ChatMessage
{
public:
    ChatMessage() = default;
    ChatMessage(int senderId,
                const QString& senderName,
                const QString& text,
                const QDateTime& time = QDateTime::currentDateTime());

    // getteri
    int       senderId()   const { return m_senderId; }
    QString   senderName() const { return m_senderName; }
    QString   text()       const { return m_text; }
    QDateTime time()       const { return m_time; }

    QJsonObject toJson() const;
    static ChatMessage fromJson(const QJsonObject& obj);

private:
    int       m_senderId = -1;
    QString   m_senderName;
    QString   m_text;
    QDateTime m_time;
};

// Da bi ChatMessage mogao da bude parametar signala
// (ClientController::chatReceived(ChatMessage)).
Q_DECLARE_METATYPE(ChatMessage)

#endif // CHATMESSAGE_H
