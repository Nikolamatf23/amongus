#ifndef TASK_H
#define TASK_H

// ============================================================================
//  Task  «abstract»   (model)
//  Stanje jednog zadatka igraca: gde je stanica, kako se zove i da li je
//  zavrsen. Ovo je jedino mesto u modelu koje zna za sve vrste taskova
//  (fabrika + pretvaranje tipa u/iz stringa), pa GameState i GameController
//  ne moraju da ukljucuju konkretne klase (WireTask, ...).
//  JSON za task pravi/cita GameState (taskToJson / taskFromJson).
//
//  Task namerno NE zna nista o UI-ju: server (GameController) drzi Task
//  objekte svih igraca i nikad ih ne prikazuje. Minigru crta TaskWidget.
//
//  Nasledjuje QObject zbog deleteLater() - GameState brise task odlozeno,
//  jer moze biti bas otvoren u TaskDialog-u.
// ============================================================================

#include <QObject>
#include <QPointF>
#include <QString>

enum class TaskType
{
    WireTask
    // ovde se dodaju nove vrste
};

class Task : public QObject
{
    Q_OBJECT

public:
    ~Task() override = default;

    // Izvedena klasa kaze koje je vrste.
    virtual TaskType type() const = 0;

    int     id() const;
    QString name() const;
    QPointF stationPos() const;
    bool    isCompleted() const;

    // Zove je samo server (GameController) i GameState::fromJson kad stigne
    // potvrda sa servera. Drugi poziv nema efekta.
    void markCompleted();

    // Fabrika: koristi je GameController::assignTasks i GameState
    // (taskFromJson). Jedino mesto koje pravi konkretne klase taskova.
    static Task* create(TaskType type,
                        int id,
                        const QString& name,
                        const QPointF& stationPos,
                        QObject* parent = nullptr);

    // Ime vrste u JSON protokolu ("WireTask"). Koristi GameState.
    static QString typeToString(TaskType type);
    static bool    typeFromString(const QString& text, TaskType& out);

protected:
    // Task se pravi samo preko create()/fromJson() ili iz izvedene klase.
    Task(int id,
         const QString& name,
         const QPointF& stationPos,
         QObject* parent = nullptr);

private:
    int     m_id;
    QString m_name;
    QPointF m_stationPos;
    bool    m_completed = false;
};

#endif // TASK_H