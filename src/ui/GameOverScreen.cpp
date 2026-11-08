#include "ui/GameOverScreen.h"

#include <QLabel>
#include <QListWidget>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

GameOverScreen::GameOverScreen(QWidget* parent)
    : QWidget(parent)
    , m_winnerLabel(new QLabel(this))
    , m_roleList(new QListWidget(this))
    , m_backBtn(new QPushButton(tr("Nazad na meni"), this))
{
    QFont f = m_winnerLabel->font();
    f.setPointSize(24);
    f.setBold(true);
    m_winnerLabel->setFont(f);
    m_winnerLabel->setAlignment(Qt::AlignCenter);

    m_roleList->setSelectionMode(QAbstractItemView::NoSelection);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_winnerLabel);
    layout->addWidget(m_roleList);
    layout->addWidget(m_backBtn, 0, Qt::AlignRight);

    connect(m_backBtn, &QPushButton::clicked, this, &GameOverScreen::backToMenuClicked);
}

void GameOverScreen::showResult(Winner w, const QList<Player>& players)
{
    switch (w) {
    case Winner::Crewmates:
        m_winnerLabel->setText(tr("Posada je pobedila!"));
        m_winnerLabel->setStyleSheet(QStringLiteral("color: #2a7de1;"));
        break;
    case Winner::Impostors:
        m_winnerLabel->setText(tr("Impostori su pobedili!"));
        m_winnerLabel->setStyleSheet(QStringLiteral("color: #d22;"));
        break;
    case Winner::None:
        m_winnerLabel->setText(tr("Kraj partije"));
        m_winnerLabel->setStyleSheet(QString());
        break;
    }

    m_roleList->clear();
    for (const Player& p : players) {
        QPixmap swatch(14, 14);
        swatch.fill(p.color().isValid() ? p.color() : Qt::gray);

        QString text = QStringLiteral("%1 - %2")
                           .arg(p.name(),
                                p.isImpostor() ? tr("Impostor") : tr("Crewmate"));
        if (!p.isAlive()) {
            text += tr(" (mrtav)");
        }

        auto* item = new QListWidgetItem(QIcon(swatch), text);
        if (p.isImpostor()) {
            item->setForeground(QColor(0xdd, 0x22, 0x22));
        }
        m_roleList->addItem(item);
    }
}
