#include "game/Task.h"

Task::Task(int id,
           const QString& name,
           const QPointF& stationPos,
           QObject* parent)
    : QObject(parent)
    , m_id(id)
    , m_name(name)
    , m_stationPos(stationPos)
{
}

int Task::id() const
{
    return m_id;
}

QString Task::name() const
{
    return m_name;
}

QPointF Task::stationPos() const
{
    return m_stationPos;
}

bool Task::isCompleted() const
{
    return m_completed;
}

void Task::markCompleted()
{
    if (m_completed) {
        return;               // signal samo jednom
    }
    m_completed = true;
    emit completed(m_id);
}
