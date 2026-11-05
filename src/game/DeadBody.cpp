#include "game/DeadBody.h"

DeadBody::DeadBody(int victimId, const QPointF& pos)
    : m_victimId(victimId)
    , m_pos(pos)
{
}

int DeadBody::victimId() const
{
    return m_victimId;
}

QPointF DeadBody::pos() const
{
    return m_pos;
}
