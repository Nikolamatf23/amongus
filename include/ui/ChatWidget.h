#ifndef CHATWIDGET_H
#define CHATWIDGET_H

// ============================================================================
//  ChatWidget
//  Chat tokom sastanka. Deo VotingScreen-a.
//
//  Dijagram:
//    - m_history: QTextEdit*
//    - m_input: QLineEdit*
//    + appendMessage(msg: ChatMessage)
//    + clear()
//    + setInputEnabled(bool)
//    <<signal>> messageSubmitted(text)
// ============================================================================

#include "game/ChatMessage.h"

#include <QWidget>

class QLineEdit;
class QTextEdit;

class ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(QWidget* parent = nullptr);

    void appendMessage(const ChatMessage& msg);
    void clear();
    void setInputEnabled(bool enabled);

signals:
    void messageSubmitted(const QString& text);

private:
    QTextEdit* m_history;
    QLineEdit* m_input;
};

#endif // CHATWIDGET_H
