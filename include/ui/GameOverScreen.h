#ifndef GAMEOVERSCREEN_H
#define GAMEOVERSCREEN_H

// ============================================================================
//  GameOverScreen
//  Pobednik i otkrivene uloge svih igraca.
//
//  Dijagram:
//    - m_winnerLabel: QLabel*
//    - m_roleList: QListWidget*
//    + showResult(w: Winner, players)
// ============================================================================

#include "game/GameTypes.h"
#include "game/Player.h"

#include <QList>
#include <QWidget>

class QLabel;
class QListWidget;
class QPushButton;

class GameOverScreen : public QWidget
{
    Q_OBJECT

public:
    explicit GameOverScreen(QWidget* parent = nullptr);

    void showResult(Winner w, const QList<Player>& players);

signals:
    void backToMenuClicked();

private:
    QLabel*      m_winnerLabel;
    QListWidget* m_roleList;
    QPushButton* m_backBtn;
};

#endif // GAMEOVERSCREEN_H
