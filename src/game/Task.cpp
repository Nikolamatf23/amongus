#include "game/Task.h"
#include "game/tasks/WireTask.h"

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
    m_completed = true;
}

// ---------------------------------------------------------------- fabrika

Task* Task::create(TaskType type,
                   int id,
                   const QString& name,
                   const QPointF& stationPos,
                   QObject* parent)
{
    switch (type) {
    case TaskType::WireTask:
        return new WireTask(id, name, stationPos, parent);
    }
    return nullptr;
}

QString Task::typeToString(TaskType type)
{
    switch (type) {
    case TaskType::WireTask:
        return QStringLiteral("WireTask");
    }
    return QString();
}

bool Task::typeFromString(const QString& text, TaskType& out)
{
    if (text == QLatin1String("WireTask")) {
        out = TaskType::WireTask;
        return true;
    }
    return false;
}
































