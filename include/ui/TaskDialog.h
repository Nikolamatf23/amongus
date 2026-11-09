#ifndef TASKDIALOG_H
#define TASKDIALOG_H

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
    Task* m_task = nullptr;
    ClientController* m_controller = nullptr;
    QWidget* m_content = nullptr;
};

#endif // TASKDIALOG_H
