#ifndef TASK_H
#define TASK_H



#include <QObject>
#include <QPointF>
#include <QString>
#include <QJsonObject>

enum class TaskType
{
    WireTask
    // za sad ovoliko
};

class Task : public QObject
{
    Q_OBJECT

public:
    Task(int id,
         const QString& name,
         const QPointF& stationPos,
         QObject* parent = nullptr);

    ~Task() override = default;


    // izvedena klasa ce mi reci koji je tip taska zapravo
    virtual TaskType type() const=0;

    // getteri
    int     id() const;
    QString name() const;
    QPointF stationPos() const;
    bool isCompleted() const;

    // Postavlja m_completed i emituje completed(id) - samo prvi put.
    void markCompleted();

    QJsonObject toJson() const;

    // Fabrika obrazac
    // Ovde pravimo Task zadate vrste

    static Task* create(TaskType type,
                            int id,
                            const QString& name,
                            const QPointF& stationPos,
                            QObject* parent=nullptr);

    // iz tipa taska izvlaci string za toJson
    static QString typeToString(TaskType type);
    // iz stringa izvlaci tip taska
    static bool typeFromString(const QString& text,TaskType& out);


signals:
    void completed(int taskId);



private:
    int     m_id;
    QString m_name;
    QPointF m_stationPos;
    bool    m_completed = false;
};

#endif // TASK_H

