#ifndef DEADBODY_H
#define DEADBODY_H

// ============================================================================
//  DeadBody
//  Telo ubijenog igraca. ID tela = ID zrtve.
//
//  Dijagram:
//    - m_victimId: int
//    - m_pos: QPointF
//    + victimId(): int
//    + pos(): QPointF
// ============================================================================

#include <QPointF>

class DeadBody
{
public:
    DeadBody() = default;
    DeadBody(int victimId, const QPointF& pos);

    int     victimId() const;
    QPointF pos() const;

private:
    int     m_victimId = -1;
    QPointF m_pos;
};

#endif // DEADBODY_H
