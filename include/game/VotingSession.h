#ifndef VOTINGSESSION_H
#define VOTINGSESSION_H

// ============================================================================
//  VotingSession
//  Jedan sastanak. GameController ga pravi u startMeeting(callerId, bodyId)
//  i brise u endMeeting(); postoji samo dok traje sastanak (0..1).
//  Tajmer sastanka je u GameController-u (m_meetingTimer).
//
//  Dijagram:
//    - m_callerId: int               - m_bodyId: int
//    - m_votes: QMap<int, int>       - m_eligible: QSet<int>
//    - m_chat: QVector<ChatMessage>
//    + castVote(voterId, targetId): bool
//    + skipVote(voterId)
//    + hasVoted(id): bool
//    + allVoted(): bool
//    + addChat(msg: ChatMessage)
//    + chatLog(): QVector<ChatMessage>
//    + tally(): int
//
//  Dogovor: targetId -1 = Skip, bodyId -1 = hitan sastanak,
//  tally() vraca -1 kada niko nije izbacen.
// ============================================================================

#include "game/ChatMessage.h"

#include <QMap>
#include <QSet>
#include <QVector>

class VotingSession
{
public:
    // eligible = ID-jevi ZIVIH igraca u trenutku sazivanja sastanka.
    VotingSession(int callerId, int bodyId, const QSet<int>& eligible);

    // false ako: glasac nije ziv/na sastanku, vec je glasao,
    // ili meta nije ziv igrac sa sastanka (a nije ni Skip = -1).
    bool castVote(int voterId, int targetId);
    void skipVote(int voterId);

    bool hasVoted(int id) const;
    bool allVoted() const;

    // Prihvata samo nepraznu poruku igraca koji je na sastanku.
    void addChat(const ChatMessage& msg);
    QVector<ChatMessage> chatLog() const;

    // ID izbacenog igraca, ili -1 (nereseno, Skip pobedio, niko nije glasao).
    int tally() const;

private:
    int                  m_callerId;
    int                  m_bodyId;
    QMap<int, int>       m_votes;      // glasac -> meta (-1 = Skip)
    QSet<int>            m_eligible;
    QVector<ChatMessage> m_chat;
};

#endif // VOTINGSESSION_H
