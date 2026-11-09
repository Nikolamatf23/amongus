#include "ui/TaskDialog.h"

#include "game/Task.h"
#include "network/ClientController.h"

#include <QVBoxLayout>
#include <QMessageBox>

TaskDialog::TaskDialog(Task* task,
                       ClientController* controller,
                       QWidget* parent)
    : QDialog(parent),
    m_task(task),
    m_controller(controller)
{
    setModal(true);

    if (m_task == nullptr) {
        setWindowTitle(tr("Task"));
        reject();
        return;
    }

    setWindowTitle(m_task->name());

    m_content = m_task->createWidget();

    if (m_content == nullptr) {
        auto* layout = new QVBoxLayout(this);
        layout->addWidget(new QMessageBox(
            QMessageBox::Warning,
            tr("Task"),
            tr("This task has no available interface."),
            QMessageBox::Ok,
            this));
        return;
    }

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_content);

    connect(m_task, &Task::completed,
            this, [this](int completedTaskId) {
                if (m_controller != nullptr) {
                    m_controller->completeTask(completedTaskId);
                }
                accept();
            });
}
