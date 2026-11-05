#include "game/VotingSession.h"

namespace
{
constexpr int SKIP = -1;   // targetId za Skip i povratna vrednost "niko"
}

VotingSession::VotingSession(int callerId, int bodyId, const QSet<int>& eligible)
    : m_callerId(callerId)
    , m_bodyId(bodyId)
    , m_eligible(eligible)
{
}

bool VotingSession::castVote(int voterId, int targetId)
{
    if (!m_eligible.contains(voterId)) {
        return false;                       // mrtav ili nije na sastanku
    }
    if (m_votes.contains(voterId)) {
        return false;                       // glas se ne menja
    }
    if (targetId != SKIP && !m_eligible.contains(targetId)) {
        return false;                       // za mrtvog/nepostojeceg ne moze
    }
    m_votes.insert(voterId, targetId);
    return true;
}

void VotingSession::skipVote(int voterId)
{
    castVote(voterId, SKIP);
}

bool VotingSession::hasVoted(int id) const
{
    return m_votes.contains(id);
}

bool VotingSession::allVoted() const
{
    for (int id : m_eligible) {
        if (!m_votes.contains(id)) {
            return false;
        }
    }
    return true;
}

void VotingSession::addChat(const ChatMessage& msg)
{
    if (m_eligible.contains(msg.senderId()) && !msg.text().isEmpty()) {
        m_chat.append(msg);
    }
}

QVector<ChatMessage> VotingSession::chatLog() const
{
    return m_chat;
}

int VotingSession::tally() const
{
    // 1) Broj glasova po meti (Skip se broji kao meta -1).
    QMap<int, int> counts;
    for (int target : m_votes) {
        counts[target] += 1;
    }

    // 2) Meta sa najvise glasova; pamtimo da li je nereseno.
    int  best = SKIP;
    int  bestCount = 0;
    bool tie = false;
    for (auto it = counts.cbegin(); it != counts.cend(); ++it) {
        if (it.value() > bestCount) {
            best = it.key();
            bestCount = it.value();
            tie = false;
        } else if (it.value() == bestCount) {
            tie = true;
        }
    }

    // 3) Nereseno, Skip pobedio ili niko nije glasao -> niko ne leti.
    if (bestCount == 0 || tie) {
        return SKIP;
    }
    return best;   // ako je pobedio Skip, best je vec -1
}
