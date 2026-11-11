#ifndef TASKDIALOG_H
#define TASKDIALOG_H

// ============================================================================
//  TaskDialog   (UI)
//  Prozor u kome se igra minigra. GameScreen ga otvara na dugme Use.
//  Pita TaskWidget::create(task) koju minigru da prikaze; kad widget javi
//  solved(), salje serveru ClientController::completeTask(id) i zatvara se.
//  Task (model) ne menja - zavrsetak potvrdjuje server.
// ============================================================================

#include <QDialog>

class Task;
class ClientController;
class QWidget;

class TaskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TaskDialog(Task* task,
                        ClientController* controller,
                        QWidget* parent = nullptr);

private:
    int               m_taskId = -1;     // pamtimo id - Task moze biti obrisan
    ClientController* m_controller = nullptr;
};

#endif // TASKDIALOG_H
