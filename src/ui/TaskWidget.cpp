#include "ui/TaskWidget.h"
#include "ui/WireTaskWidget.h"

#include "game/Task.h"
#include "game/tasks/WireTask.h"

TaskWidget::TaskWidget(QWidget* parent)
    : QWidget(parent)
{
}

TaskWidget* TaskWidget::create(const Task& task, QWidget* parent)
{
    switch (task.type()) {
    case TaskType::WireTask:
        return new WireTaskWidget(WireTask::WIRE_COUNT, parent);
    }
    return nullptr;
}
