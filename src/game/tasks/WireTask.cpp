#include "game/tasks/WireTask.h"

WireTask::WireTask(int id,
                   const QString& name,
                   const QPointF& stationPos,
                   QObject* parent)
    : Task(id, name, stationPos, parent)
{
}

TaskType WireTask::type() const
{
    return TaskType::WireTask;
}