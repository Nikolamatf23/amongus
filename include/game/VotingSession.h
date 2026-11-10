#ifndef VOTINGSESSION_H
#define VOTINGSESSION_H

#include "game/ChatMessage.h"

#include <QMap>
#include <QSet>
#include <QVector>

// Cuva glasove i poruke jednog sastanka.
// GameController pokrece sastanak, vodi tajmer i primenjuje rezultat.
class VotingSession
{
public:
    // bodyId je -1 za hitan sastanak. eligible sadrzi ID-jeve zivih igraca.
    VotingSession(int callerId, int bodyId, const QSet<int>& eligible);

    // Igrac moze da glasa samo jednom. targetId == -1 oznacava Skip.
    bool castVote(int voterId, int targetId);
    void skipVote(int voterId);

    bool hasVoted(int id) const;
    // Proverava da li su svi igraci koji su jos na sastanku glasali.
    bool allVoted() const;

    // Prihvata neprazne poruke igraca koji mogu da ucestvuju na sastanku.
    void addChat(const ChatMessage& msg);
    QVector<ChatMessage> chatLog() const;

    // Vraca ID izbacenog igraca. Ako pobedi Skip, rezultat je neresen
    // ili nema vazecih glasova, vraca -1.
    int tally() const;

    // Uklanja igraca iz glasanja i brise glas koji je on dao.
    void removePlayer(int playerId);

private:
    static constexpr int Skip = -1;

    int m_callerId;
    int m_bodyId;
    QMap<int, int> m_votes; // ID glasaca -> ID mete ili Skip
    QSet<int> m_eligible;
    QVector<ChatMessage> m_chat;
};

#endif // VOTINGSESSION_H
