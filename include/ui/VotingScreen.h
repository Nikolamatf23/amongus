#ifndef VOTINGSCREEN_H
#define VOTINGSCREEN_H

// ============================================================================
//  VotingScreen
//  Sastanak: lista igraca za glasanje, Skip, odbrojavanje i chat.
//
//  Dijagram:
//    - m_playerList: QListWidget*    - m_skipBtn: QPushButton*
//    - m_timerLabel: QLabel*         - m_chat: ChatWidget*
//    - m_countdown: QTimer
//    + setup(players)
//    + setVotingEnabled(bool)
//    + showResult(ejectedId, wasImpostor)
// ============================================================================

#include "game/Player.h"

#include <QList>
#include <QTimer>
#include <QWidget>

class ChatWidget;
class QLabel;
class QListWidget;
class QPushButton;

class VotingScreen : public QWidget
{
    Q_OBJECT

public:
    explicit VotingScreen(QWidget* parent = nullptr);

    // Puni listu igraca i pokrece odbrojavanje od 'seconds' sekundi.
    // Mrtvi igraci su sivi i ne mogu se izabrati; mrtav igrac ne glasa.
    void setup(const QList<Player>& players, int myId, int seconds);

    void setVotingEnabled(bool enabled);
    void showResult(int ejectedId, bool wasImpostor);

    ChatWidget* chat() const;

signals:
    void voteCast(int targetId);   // -1 = Skip

private slots:
    void onTick();

private:
    void castVote(int targetId);
    void updateTimerLabel();

    QListWidget* m_playerList;
    QPushButton* m_voteBtn;
    QPushButton* m_skipBtn;
    QLabel*      m_timerLabel;
    QLabel*      m_resultLabel;
    ChatWidget*  m_chat;
    QTimer       m_countdown;

    QList<Player> m_players;
    int  m_secondsLeft = 0;
    bool m_iAmAlive = true;
    bool m_hasVoted = false;
};

#endif // VOTINGSCREEN_H
