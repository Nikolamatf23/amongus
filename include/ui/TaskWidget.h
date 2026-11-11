#ifndef TASKWIDGET_H
#define TASKWIDGET_H

// ============================================================================
//  TaskWidget  «abstract»   (UI)
//  Osnovna klasa za minigre taskova. Svaka vrsta taska ima svoj widget
//  (WireTaskWidget, ...). Widget samo prikazuje minigru i javlja kad je
//  igrac resi - NE menja model (ne zove Task::markCompleted()).
//
//    + static create(task, parent): TaskWidget*   - bira widget po task.type()
//    «signal» solved()                            - igrac je resio minigru
//
//  TaskDialog povezuje solved() -> ClientController::completeTask(id).
// ============================================================================

#include <QWidget>

class Task;

class TaskWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskWidget(QWidget* parent = nullptr);
    ~TaskWidget() override = default;

    // Pravi minigru za dati task; nullptr ako vrsta nema widget.
    // Jedino mesto u UI-ju koje zna za sve vrste taskova.
    static TaskWidget* create(const Task& task, QWidget* parent = nullptr);

signals:
    void solved();
};

#endif // TASKWIDGET_H
