#include "ui/StartScreen.h"

#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

StartScreen::StartScreen(QWidget* parent)
    : QWidget(parent)
    , m_nameEdit(new QLineEdit(this))
    , m_ipEdit(new QLineEdit(this))
    , m_hostBtn(new QPushButton(tr("Host"), this))
    , m_joinBtn(new QPushButton(tr("Join"), this))
    , m_background(QStringLiteral(":/ui/startScreenBackground.png"))
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

    // Forma je u poluprovidnom tamnom panelu da bi se citala preko slike.
    auto* panel = new QFrame(this);
    panel->setObjectName(QStringLiteral("startPanel"));
    panel->setStyleSheet(QStringLiteral(
        "#startPanel { background-color: rgba(0, 0, 0, 170); border-radius: 12px; }"
        "#startPanel QLabel { color: white; }"));

    auto* box = new QVBoxLayout(panel);
    box->setContentsMargins(30, 24, 30, 24);
    box->addWidget(title);
    box->addSpacing(20);
    box->addLayout(form);
    box->addLayout(buttons);

    // Panel je centriran i ne siri se preko celog prozora.
    auto* outer = new QVBoxLayout(this);
    outer->addStretch();
    outer->addWidget(panel, 0, Qt::AlignHCenter);
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

void StartScreen::paintEvent(QPaintEvent* event)
{
    QPainter p(this);

    if (m_background.isNull()) {
        // Slika nije ugradjena (nema je u resources.qrc) - samo crna pozadina.
        p.fillRect(rect(), Qt::black);
    } else {
        // Skaliranje je skupo (2560x1440), pa se radi samo kad se promeni velicina ekrana.
        if (m_scaledFor != size()) {
            m_scaled = m_background.scaled(size(), Qt::KeepAspectRatioByExpanding,
                                           Qt::SmoothTransformation);
            m_scaledFor = size();
        }
        // Centriranje: visak slike se ravnomerno odseca sa obe strane.
        const int x = (width()  - m_scaled.width())  / 2;
        const int y = (height() - m_scaled.height()) / 2;
        p.drawPixmap(x, y, m_scaled);
    }

    QWidget::paintEvent(event);
}

void StartScreen::updateButtons()
{
    const bool hasName = !m_nameEdit->text().trimmed().isEmpty();
    const bool hasIp   = !m_ipEdit->text().trimmed().isEmpty();
    m_hostBtn->setEnabled(!m_busy && hasName);
    m_joinBtn->setEnabled(!m_busy && hasName && hasIp);
}
