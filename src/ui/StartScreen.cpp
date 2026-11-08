#include "ui/StartScreen.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

StartScreen::StartScreen(QWidget* parent)
    : QWidget(parent)
    , m_nameEdit(new QLineEdit(this))
    , m_ipEdit(new QLineEdit(this))
    , m_hostBtn(new QPushButton(tr("Host"), this))
    , m_joinBtn(new QPushButton(tr("Join"), this))
{
    auto* title = new QLabel(tr("Among Us"), this);
    QFont f = title->font();
    f.setPointSize(28);
    f.setBold(true);
    title->setFont(f);
    title->setAlignment(Qt::AlignCenter);

    m_nameEdit->setPlaceholderText(tr("Ime igraca"));
    m_nameEdit->setMaxLength(16);
    m_ipEdit->setPlaceholderText(tr("IP adresa hosta"));
    m_ipEdit->setText(QStringLiteral("127.0.0.1"));

    auto* form = new QFormLayout;
    form->addRow(tr("Ime:"), m_nameEdit);
    form->addRow(tr("IP:"), m_ipEdit);

    auto* buttons = new QHBoxLayout;
    buttons->addWidget(m_hostBtn);
    buttons->addWidget(m_joinBtn);

    auto* box = new QVBoxLayout;
    box->addWidget(title);
    box->addSpacing(20);
    box->addLayout(form);
    box->addLayout(buttons);

    // Forma je centrirana i ne siri se preko celog prozora.
    auto* outer = new QHBoxLayout(this);
    outer->addStretch();
    outer->addLayout(box);
    outer->addStretch();

    connect(m_nameEdit, &QLineEdit::textChanged, this, &StartScreen::updateButtons);
    connect(m_ipEdit,   &QLineEdit::textChanged, this, &StartScreen::updateButtons);

    connect(m_hostBtn, &QPushButton::clicked, this, [this] {
        emit hostClicked(m_nameEdit->text().trimmed());
    });
    connect(m_joinBtn, &QPushButton::clicked, this, [this] {
        emit joinClicked(m_nameEdit->text().trimmed(), m_ipEdit->text().trimmed());
    });

    updateButtons();
}

void StartScreen::setBusy(bool busy)
{
    m_busy = busy;
    updateButtons();
}

void StartScreen::updateButtons()
{
    const bool hasName = !m_nameEdit->text().trimmed().isEmpty();
    const bool hasIp   = !m_ipEdit->text().trimmed().isEmpty();
    m_hostBtn->setEnabled(!m_busy && hasName);
    m_joinBtn->setEnabled(!m_busy && hasName && hasIp);
}
