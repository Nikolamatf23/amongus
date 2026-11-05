#ifndef PLAYER_H
#define PLAYER_H

// ============================================================================
//  Player
//  Jedan igrac. GameState ih cuva u QMap<int, Player> (4..10 igraca).
//
//  Dijagram:
//    - m_id: int              - m_name: QString       - m_color: QColor
//    - m_pos: QPointF         - m_inputDir: QPointF   - m_role: Role
//    - m_alive: bool          - m_killCooldown: double
//    - m_tasks: QVector<Task*>
//    + isImpostor(): bool
//    + resetKillCooldown()
//    + tickCooldown(dt)
//    + assignTask(t: Task*)
//    + completedTasks(): int
//    + getteri / setteri
//
//  m_tasks su samo pokazivaci (agregacija): Player se kopira, pa taskove
//  ne brise on, nego GameState (u destruktoru i u removePlayer()).
// ============================================================================

#include "game/GameTypes.h"

#include <QColor>
#include <QPointF>
#include <QString>
#include <QVector>

class Task;

class Player
{
public:
    Player() = default;
    Player(int id, const QString& name, const QColor& color = QColor());

    bool isImpostor() const;

    // Ponovo pokrece cooldown za ubijanje (posle ubistva).
    void resetKillCooldown();

    // Smanjuje cooldown za dt sekundi (ne ispod 0).
    void tickCooldown(double dt);

    void assignTask(Task* t);
    int  completedTasks() const;

    // ---- getteri ----
    int                   id() const;
    QString               name() const;
    QColor                color() const;
    QPointF               pos() const;
    QPointF               inputDir() const;
    Role                  role() const;
    bool                  isAlive() const;
    double                killCooldown() const;
    const QVector<Task*>& tasks() const;

    // ---- setteri ----
    void setName(const QString& name);
    void setColor(const QColor& color);
    void setPos(const QPointF& pos);
    void setInputDir(const QPointF& dir);
    void setRole(Role role);
    void setAlive(bool alive);
    void setKillCooldown(double seconds);
    void setTasks(const QVector<Task*>& tasks);

private:
    int            m_id = -1;
    QString        m_name;
    QColor         m_color;
    QPointF        m_pos;
    QPointF        m_inputDir;
    Role           m_role = Role::Crewmate;
    bool           m_alive = true;
    double         m_killCooldown = 0.0;
    QVector<Task*> m_tasks;
};

#endif // PLAYER_H
