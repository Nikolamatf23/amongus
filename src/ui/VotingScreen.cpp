#include "ui/VotingScreen.h"
#include "ui/ChatWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

VotingScreen::VotingScreen(QWidget* parent)
    : QWidget(parent)
    , m_playerList(new QListWidget(this))
    , m_voteBtn(new QPushButton(tr("Glasaj"), this))
    , m_skipBtn(new QPushButton(tr("Skip"), this))
    , m_timerLabel(new QLabel(this))
    , m_resultLabel(new QLabel(this))
    , m_chat(new ChatWidget(this))
{
    auto* title = new QLabel(tr("Sastanak"), this);
    QFont f = title->font();
    f.setPointSize(20);
    f.setBold(true);
    title->setFont(f);

    m_resultLabel->setWordWrap(true);
    m_playerList->setSelectionMode(QAbstractItemView::SingleSelection);

    auto* header = new QHBoxLayout;
    header->addWidget(title);
    header->addStretch();
    header->addWidget(m_timerLabel);

    auto* buttons = new QHBoxLayout;
    buttons->addWidget(m_voteBtn);
    buttons->addWidget(m_skipBtn);

    auto* left = new QVBoxLayout;
    left->addWidget(m_playerList);
    left->addLayout(buttons);

    auto* body = new QHBoxLayout;
    body->addLayout(left, 1);
    body->addWidget(m_chat, 1);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(header);
    layout->addLayout(body);
    layout->addWidget(m_resultLabel);

    m_countdown.setInterval(1000);
    connect(&m_countdown, &QTimer::timeout, this, &VotingScreen::onTick);

    connect(m_voteBtn, &QPushButton::clicked, this, [this] {
        if (QListWidgetItem* item = m_playerList->currentItem()) {
            castVote(item->data(Qt::UserRole).toInt());
        }
    });
    connect(m_skipBtn, &QPushButton::clicked, this, [this] { castVote(-1); });
    connect(m_playerList, &QListWidget::itemDoubleClicked, this,
            [this](QListWidgetItem* item) { castVote(item->data(Qt::UserRole).toInt()); });

    setVotingEnabled(false);
}

void VotingScreen::setup(const QList<Player>& players, int myId, int seconds)
{
    m_players     = players;
    m_hasVoted    = false;
    m_iAmAlive    = true;
    m_secondsLeft = seconds;
    m_resultLabel->clear();
    m_playerList->clear();

    for (const Player& p : players) {
        if (p.id() == myId) {
            m_iAmAlive = p.isAlive();
        }

        QPixmap swatch(14, 14);
        swatch.fill(Qt::gray);

        QString text = p.name();
        if (p.id() == myId)  text += tr(" (ti)");
        if (!p.isAlive())    text += tr(" - mrtav");

        auto* item = new QListWidgetItem(QIcon(swatch), text);
        item->setData(Qt::UserRole, p.id());
        if (!p.isAlive()) {
            // Za mrtve ne moze da se glasa.
            item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
        }
        m_playerList->addItem(item);
    }

    // Mrtvi mogu samo da citaju chat.
    m_chat->setInputEnabled(m_iAmAlive);

    updateTimerLabel();
    if (seconds > 0) {
        m_countdown.start();
    } else {
        m_countdown.stop();
    }
}

void VotingScreen::setVotingEnabled(bool enabled)
{
    const bool on = enabled && m_iAmAlive && !m_hasVoted;
    m_playerList->setEnabled(on);
    m_voteBtn->setEnabled(on);
    m_skipBtn->setEnabled(on);
}

void VotingScreen::showResult(int ejectedId, bool wasImpostor)
{
    m_countdown.stop();
    setVotingEnabled(false);

    QString name;
    for (const Player& p : m_players) {
        if (p.id() == ejectedId) {
            name = p.name();
            break;
        }
    }

    if (ejectedId < 0 || name.isEmpty()) {
        m_resultLabel->setText(tr("Niko nije izbacen."));
    } else if (wasImpostor) {
        m_resultLabel->setText(tr("%1 je izbacen. Bio je impostor!").arg(name));
    } else {
        m_resultLabel->setText(tr("%1 je izbacen. Nije bio impostor.").arg(name));
    }
}

ChatWidget* VotingScreen::chat() const
{
    return m_chat;
}

void VotingScreen::onTick()
{
    // Odbrojavanje je samo prikaz; kraj glasanja odlucuje server.
    if (m_secondsLeft > 0) {
        --m_secondsLeft;
    }
    updateTimerLabel();
    if (m_secondsLeft == 0) {
        m_countdown.stop();
    }
}

void VotingScreen::castVote(int targetId)
{
    if (m_hasVoted || !m_iAmAlive) {
        return;
    }
    m_hasVoted = true;
    setVotingEnabled(false);
    emit voteCast(targetId);
}

void VotingScreen::updateTimerLabel()
{
    m_timerLabel->setText(QStringLiteral("%1:%2")
                              .arg(m_secondsLeft / 60)
                              .arg(m_secondsLeft % 60, 2, 10, QLatin1Char('0')));
}
