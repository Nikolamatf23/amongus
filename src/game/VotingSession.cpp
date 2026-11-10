#include "game/VotingSession.h"

VotingSession::VotingSession(int callerId, int bodyId, const QSet<int>& eligible)
    : m_callerId(callerId)
    , m_bodyId(bodyId)
    , m_eligible(eligible)
{
}

bool VotingSession::castVote(int voterId, int targetId)
{
    if (!m_eligible.contains(voterId) || hasVoted(voterId)) {
        return false;
    }

    // Meta mora biti igrac koji je jos na sastanku. Skip nema metu.
    if (targetId != Skip && !m_eligible.contains(targetId)) {
        return false;
    }

    m_votes.insert(voterId, targetId);
    return true;
}

void VotingSession::skipVote(int voterId)
{
    castVote(voterId, Skip);
}

bool VotingSession::hasVoted(int id) const
{
    return m_votes.contains(id);
}

bool VotingSession::allVoted() const
{
    // Svaki igrac koji je jos na sastanku moze imati najvise jedan glas.
    // Glas igraca koji je otisao uklanja se u removePlayer().
    return m_votes.size() == m_eligible.size();
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
    // Skip brojimo uz ostale opcije. Glasove za igrace koji su otisli
    // izostavljamo iz rezultata.
    QMap<int, int> counts;
    for (int targetId : m_votes) {
        if (targetId == Skip || m_eligible.contains(targetId)) {
            ++counts[targetId];
        }
    }

    constexpr int NoEjection = -1;
    int result = NoEjection;
    int highestCount = 0;

    for (auto it = counts.cbegin(); it != counts.cend(); ++it) {
        if (it.value() > highestCount) {
            // Novi maksimum razresava prethodno izjednacenje.
            highestCount = it.value();
            result = it.key();
        } else if (it.value() == highestCount) {
            result = NoEjection;
        }
    }

    return result;
}

void VotingSession::removePlayer(int playerId)
{
    // Ostali glasaci zadrzavaju status da su glasali, jer im UI vec
    // zakljucava dugmad. tally() izostavlja njihove glasove za ovu metu.
    m_eligible.remove(playerId);
    m_votes.remove(playerId);
}
