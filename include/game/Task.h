#ifndef TASK_H
#define TASK_H

// ============================================================================
//  Task  «abstract»
//  Osnovna klasa svih zadataka. Svaki task je na jednoj stanici na mapi.
//
//  Dijagram:
//    - m_id: int                 - m_name: QString
//    - m_stationPos: QPointF     - m_completed: bool
//    + isCompleted(): bool
//    + markCompleted()
//    + createWidget(): QWidget*  {abstract}
//    «signal» completed(taskId)
//
//  Na klijentu TaskDialog zove createWidget() i prikazuje minigru. Kad je
//  igrac resi, podklasa zove markCompleted() -> signal completed(taskId)
//  -> ClientController::completeTask(taskId).
// ============================================================================

#include <QObject>
#include <QPointF>
#include <QString>

class QWidget;

class Task : public QObject
{
    Q_OBJECT

public:
    Task(int id,
         const QString& name,
         const QPointF& stationPos,
         QObject* parent = nullptr);
    ~Task() override = default;

    // getteri
    int     id() const;
    QString name() const;
    QPointF stationPos() const;

    bool isCompleted() const;

    // Postavlja m_completed i emituje completed(id) - samo prvi put.
    void markCompleted();

    // Pravi widget sa minigrom; vlasnik je pozivalac (TaskDialog).
    virtual QWidget* createWidget() = 0;

    signals:
        void completed(int taskId);

private:
    int     m_id;
    QString m_name;
    QPointF m_stationPos;
    bool    m_completed = false;
};

#endif // TASK_H
