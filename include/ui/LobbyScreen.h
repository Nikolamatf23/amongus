#ifndef LOBBYSCREEN_H
#define LOBBYSCREEN_H

// ============================================================================
//  LobbyScreen
//  Lista igraca koji cekaju i dugme Start (vidi ga samo host).
//
//  Dijagram:
//    - m_playerList: QListWidget*
//    - m_startBtn: QPushButton*
//    + setHost(bool)
//    + updatePlayers(names)
// ============================================================================

#include "game/Player.h"

#include <QList>
#include <QWidget>

class QLabel;
class QListWidget;
class QPushButton;

class LobbyScreen : public QWidget
{
    Q_OBJECT

public:
    explicit LobbyScreen(QWidget* parent = nullptr);

    void setHost(bool isHost);
    void updatePlayers(const QList<Player>& players);

signals:
    void startClicked();
    void leaveClicked();

private:
    QListWidget* m_playerList;
    QPushButton* m_startBtn;
    QPushButton* m_leaveBtn;
    QLabel*      m_infoLabel;
};

#endif // LOBBYSCREEN_H
