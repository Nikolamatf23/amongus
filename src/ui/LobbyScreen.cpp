#include "ui/LobbyScreen.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

LobbyScreen::LobbyScreen(QWidget* parent)
    : QWidget(parent)
    , m_playerList(new QListWidget(this))
    , m_startBtn(new QPushButton(tr("Start"), this))
    , m_leaveBtn(new QPushButton(tr("Napusti"), this))
    , m_infoLabel(new QLabel(this))
{
    auto* title = new QLabel(tr("Lobby"), this);
    QFont f = title->font();
    f.setPointSize(20);
    f.setBold(true);
    title->setFont(f);

    m_playerList->setSelectionMode(QAbstractItemView::NoSelection);

    auto* buttons = new QHBoxLayout;
    buttons->addWidget(m_leaveBtn);
    buttons->addStretch();
    buttons->addWidget(m_startBtn);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(title);
    layout->addWidget(m_infoLabel);
    layout->addWidget(m_playerList);
    layout->addLayout(buttons);

    connect(m_startBtn, &QPushButton::clicked, this, &LobbyScreen::startClicked);
    connect(m_leaveBtn, &QPushButton::clicked, this, &LobbyScreen::leaveClicked);

    setHost(false);
}

void LobbyScreen::setHost(bool isHost)
{
    // Server svakako proverava da li je posiljalac host;
    // ovde samo ne prikazujemo dugme ostalima.
    m_startBtn->setVisible(isHost);
    m_infoLabel->setText(isHost ? tr("Ti si host. Pokreni partiju kad svi udju.")
                                : tr("Cekamo da host pokrene partiju..."));
}

void LobbyScreen::updatePlayers(const QList<Player>& players)
{
    m_playerList->clear();
    for (const Player& p : players) {
        QPixmap swatch(14, 14);
        swatch.fill(p.color().isValid() ? p.color() : Qt::gray);
        m_playerList->addItem(new QListWidgetItem(QIcon(swatch), p.name()));
    }
}
