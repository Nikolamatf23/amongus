#include "game/Player.h"
#include "game/Task.h"

#include <QtMath>

namespace
{
// Mora da bude isto kao GameController::KILL_COOLDOWN.
constexpr double KILL_COOLDOWN_SECONDS = 25.0;
}

Player::Player(int id, const QString& name, const QColor& color)
    : m_id(id)
    , m_name(name)
    , m_color(color)
{
}

bool Player::isImpostor() const
{
    return m_role == Role::Impostor;
}

void Player::resetKillCooldown()
{
    m_killCooldown = KILL_COOLDOWN_SECONDS;
}

void Player::tickCooldown(double dt)
{
    m_killCooldown = qMax(0.0, m_killCooldown - dt);
}

void Player::assignTask(Task* t)
{
    if (t != nullptr && !m_tasks.contains(t)) {
        m_tasks.append(t);
    }
}

int Player::completedTasks() const
{
    int count = 0;
    for (const Task* t : m_tasks) {
        if (t != nullptr && t->isCompleted()) {
            ++count;
        }
    }
    return count;
}

// ---------------------------------------------------------------- getteri

int                   Player::id() const           { return m_id; }
QString               Player::name() const         { return m_name; }
QColor                Player::color() const        { return m_color; }
QPointF               Player::pos() const          { return m_pos; }
QPointF               Player::inputDir() const     { return m_inputDir; }
Role                  Player::role() const         { return m_role; }
bool                  Player::isAlive() const      { return m_alive; }
double                Player::killCooldown() const { return m_killCooldown; }
const QVector<Task*>& Player::tasks() const        { return m_tasks; }

// ---------------------------------------------------------------- setteri

void Player::setName(const QString& name)   { m_name = name; }
void Player::setColor(const QColor& color)  { m_color = color; }
void Player::setPos(const QPointF& pos)     { m_pos = pos; }
void Player::setRole(Role role)             { m_role = role; }
void Player::setKillCooldown(double s)      { m_killCooldown = s; }
void Player::setTasks(const QVector<Task*>& tasks) { m_tasks = tasks; }

void Player::setInputDir(const QPointF& dir)
{
    // Vektor duzi od 1 se skracuje na 1, da dijagonala ne bi bila brza
    // i da klijent ne bi mogao da ubrza slanjem velikog broja.
    const double len = qSqrt(dir.x() * dir.x() + dir.y() * dir.y());
    m_inputDir = (len > 1.0) ? dir / len : dir;
}

void Player::setAlive(bool alive)
{
    m_alive = alive;
    if (!alive) {
        m_inputDir = QPointF();   // mrtav igrac staje
    }
}
