#include "ui/ChatWidget.h"

#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>

ChatWidget::ChatWidget(QWidget* parent)
    : QWidget(parent)
    , m_history(new QTextEdit(this))
    , m_input(new QLineEdit(this))
{
    m_history->setReadOnly(true);
    m_input->setPlaceholderText(tr("Poruka... (Enter za slanje)"));
    m_input->setMaxLength(200);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_history);
    layout->addWidget(m_input);

    connect(m_input, &QLineEdit::returnPressed, this, [this] {
        const QString text = m_input->text().trimmed();
        if (text.isEmpty()) {
            return;
        }
        emit messageSubmitted(text);
        m_input->clear();
    });
}

void ChatWidget::appendMessage(const ChatMessage& msg)
{
    // Tekst escape-ujemo da igrac ne bi mogao da ubaci HTML u tudji chat.
    const QString line = QStringLiteral("<span style='color:gray'>[%1]</span> <b>%2:</b> %3")
                             .arg(msg.time().toString(QStringLiteral("HH:mm")),
                                  msg.senderName().toHtmlEscaped(),
                                  msg.text().toHtmlEscaped());
    m_history->append(line);
}

void ChatWidget::clear()
{
    m_history->clear();
    m_input->clear();
}

void ChatWidget::setInputEnabled(bool enabled)
{
    m_input->setEnabled(enabled);
}
