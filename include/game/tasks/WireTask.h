#ifndef WIRETASK_H
#define WIRETASK_H


#include "game/Task.h"


class WireTask : public Task
{
    Q_OBJECT

public:
    //broj zica za spajanje
    static constexpr int WIRE_COUNT=4;


    WireTask(int id,
             const QString& name,
             const QPointF& stationPos,
             QObject* parent=nullptr);


    TaskType type() const override;
};

#endif // WIRETASK_H
