#ifndef WIRETASK_H
#define WIRETASK_H

// ============================================================================
//  WireTask  (nasledjuje Task)
//  Minigra "spajanje zica": levo 4 zice u boji, desno iste boje izmesane.
//  Igrac misem prevuce zicu do iste boje; kad su sve 4 spojene, task se
//  zavrsava (markCompleted -> signal completed).
//
//  Dijagram:
//    - m_wireColors: QVector<QColor>
//    - m_order: QVector<int>
//    + createWidget(): QWidget*
// ============================================================================

#include "game/Task.h"

#include <QColor>
#include <QVector>

class WireTask : public Task
{
    Q_OBJECT

public:
    WireTask(int id,
             const QString& name,
             const QPointF& stationPos,
             QObject* parent = nullptr);

    QWidget* createWidget() override;

private:
    QVector<QColor> m_wireColors;   // boje zica, leva strana odozgo nadole
    QVector<int>    m_order;        // m_order[i] = boja na i-tom mestu desno
};

#endif // WIRETASK_H
