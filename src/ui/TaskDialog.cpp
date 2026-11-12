#include "ui/TaskDialog.h"
#include "ui/TaskWidget.h"

#include "game/Task.h"
#include "network/ClientController.h"

#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>

TaskDialog::TaskDialog(Task* task,
                       ClientController* controller,
                       QWidget* parent)
    : QDialog(parent)
    , m_controller(controller)
{
    setModal(true);
    auto* layout = new QVBoxLayout(this);

    if (task == nullptr) {
        setWindowTitle(tr("Task"));
        // reject() u konstruktoru ne radi pre exec(), pa ga odlazemo.
        QTimer::singleShot(0, this, &QDialog::reject);
        return;
    }

    if (m_controller != nullptr) {
        connect(m_controller, &ClientController::phaseChanged,
                this, [this](GamePhase ph) {
                    if (ph != GamePhase::Playing) {
                        reject();   // sastanak je počeo, task se prekida
                    }
                });
    }

    m_taskId = task->id();
    setWindowTitle(task->name());

    TaskWidget* content = TaskWidget::create(*task, this);
    if (content == nullptr) {
        layout->addWidget(new QLabel(tr("Ovaj task jos nema minigru."), this));
        return;
    }
    layout->addWidget(content);

    connect(content, &TaskWidget::solved, this, [this]() {
        if (m_controller != nullptr) {
            m_controller->completeTask(m_taskId);
        }
        // Kratka pauza da igrac vidi "Gotovo!", pa zatvaranje.
        QTimer::singleShot(400, this, &QDialog::accept);
    });
}
