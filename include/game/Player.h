#ifndef PLAYER_H
#define PLAYER_H


#include "game/GameTypes.h"

#include <QColor>
#include <QPointF>
#include <QString>
#include <QVector>

class Task;

class Player
{
public:

    // Koliko sekundi impostor ceka izmedju dva ubistva
    static constexpr double KILL_COOLDOWN=25.0;

    // Poluprecnik igraca u pikselima (za kolizije)
    static constexpr double RADIUS=20.0;

    Player() = default;
    Player(int id, const QString& name);

    bool isImpostor() const;

    // Impostor koji je i dalje u igri i ciji je cooldown istekao
    // Domet do mete ce proveriti server
    bool canKill() const;

    // Ponovo pokrece cooldown za ubijanje (posle ubistva).
    void resetKillCooldown();

    // Smanjuje cooldown za dt sekundi (ne ispod 0).
    void tickCooldown(double dt);


    //metode za taskove
    void assignTask(Task* t);
    Task* task(int taskId) const;
    bool hasTask(int taskId) const;
    int completedTasks() const;
    bool allTasksCompleted() const;


    double distanceTo(const QPointF& point) const;
    bool isNear(const QPointF& point,double range) const;



    // ---- getteri ----
    int                   id() const;
    QString               name() const;
    QPointF               pos() const;
    int                   skinId() const;
    QPointF               inputDir() const;
    Role                  role() const;
    bool                  isAlive() const;
    double                killCooldown() const;
    const QVector<Task*>& tasks() const;

    // ---- setteri ----
    void setName(const QString& name);
    void setSkinId(int skinId);
    void setPos(const QPointF& pos);
    void setInputDir(const QPointF& dir);
    void setRole(Role role);
    void setAlive(bool alive);
    void setKillCooldown(double seconds);
    void setTasks(const QVector<Task*>& tasks);

private:
    int            m_id = -1;
    QString        m_name;
    int            m_skinId = 1;
    QPointF        m_pos;
    QPointF        m_inputDir;
    Role           m_role = Role::Crewmate;
    bool           m_alive = true;
    double         m_killCooldown = 0.0;
    QVector<Task*> m_tasks;
};

#endif // PLAYER_H
