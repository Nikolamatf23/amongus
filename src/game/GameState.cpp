#include "game/GameState.h"
#include "game/GameMap.h"
#include "game/Task.h"
#include "game/tasks/WireTask.h"

#include <QJsonArray>
#include <QSet>

namespace
{

// Prebrojava taskove POSADE (i zive i mrtve - duhovi mogu da ih zavrse).
// Impostorovi taskovi su lazni i ne broje se.
void countTasks(const QMap<int, Player>& players, int& done, int& total)
{
    done = 0;
    total = 0;
    for (const Player& p : players) {
        if (p.isImpostor()) {
            continue;
        }
        total += p.tasks().size();
        done  += p.completedTasks();
    }
}

// Pravi task odgovarajuce klase po imenu klase iz JSON-a.
Task* createTask(const QString& type, int id, const QString& name, const QPointF& pos)
{
    if (type == QLatin1String("WireTask")) {
        return new WireTask(id, name, pos);
    }
    return nullptr;   // ovde se dodaju nove vrste taskova
}

QJsonObject taskToJson(const Task* t)
{
    QJsonObject obj;
    obj["id"]        = t->id();
    obj["type"]      = QString::fromLatin1(t->metaObject()->className());
    obj["name"]      = t->name();
    obj["x"]         = t->stationPos().x();
    obj["y"]         = t->stationPos().y();
    obj["completed"] = t->isCompleted();
    return obj;
}

QJsonObject playerToJson(const Player& p)
{
    QJsonObject obj;
    obj["id"]           = p.id();
    obj["name"]         = p.name();
    obj["color"]        = p.color().name();
    obj["x"]            = p.pos().x();
    obj["y"]            = p.pos().y();
    obj["role"]         = static_cast<int>(p.role());
    obj["alive"]        = p.isAlive();
    obj["killCooldown"] = p.killCooldown();

    QJsonArray tasks;
    for (const Task* t : p.tasks()) {
        if (t != nullptr) {
            tasks.append(taskToJson(t));
        }
    }
    obj["tasks"] = tasks;
    return obj;
}

} // namespace

// ============================================================================

GameMap* GameState::getMap() const
{
    return m_map;
}

GameState::GameState(GameMap* map)
    : m_map(map)
{
}

GameState::~GameState()
{
    for (const Player& p : m_players) {
        qDeleteAll(p.tasks());
    }
}

// ---------------------------------------------------------------- igraci

void GameState::addPlayer(const Player& p)
{
    if (p.id() < 0 || m_players.contains(p.id()) || m_players.size() >= 16) {
        return;                                // max 16 igraca (4..16)
    }
    Player copy = p;
    // Novi igrac se pojavljuje na sledecoj spawn tacki mape.
    if (m_map != nullptr && !m_map->spawnPoints().isEmpty()) {
        const QVector<QPointF> spawns = m_map->spawnPoints();
        copy.setPos(spawns[m_players.size() % spawns.size()]);
    }
    m_players.insert(copy.id(), copy);
}

void GameState::removePlayer(int id)
{
    auto it = m_players.find(id);
    if (it == m_players.end()) {
        return;
    }
    for (Task* t : it->tasks()) {
        if (t != nullptr) {
            t->deleteLater();   // mozda je bas otvoren u TaskDialog-u
        }
    }
    m_players.erase(it);
}

Player* GameState::player(int id)
{
    auto it = m_players.find(id);
    return it == m_players.end() ? nullptr : &it.value();
}

const Player* GameState::player(int id) const
{
    auto it = m_players.constFind(id);
    return it == m_players.cend() ? nullptr : &it.value();
}

QList<Player> GameState::players() const
{
    return m_players.values();
}

QList<Player> GameState::alivePlayers() const
{
    QList<Player> result;
    for (const Player& p : m_players) {
        if (p.isAlive()) {
            result.append(p);
        }
    }
    return result;
}

// ---------------------------------------------------------------- tela

void GameState::addBody(const DeadBody& b)
{
    for (const DeadBody& existing : m_bodies) {
        if (existing.victimId() == b.victimId()) {
            return;                            // jedna zrtva = jedno telo
        }
    }
    m_bodies.append(b);
}

QVector<DeadBody> GameState::bodies() const
{
    return m_bodies;
}

void GameState::clearBodies()
{
    m_bodies.clear();
}

// ---------------------------------------------------------------- faza

GamePhase GameState::phase() const
{
    return m_phase;
}

void GameState::setPhase(GamePhase ph)
{
    m_phase = ph;
}

// ---------------------------------------------------------------- taskovi i pobeda

double GameState::taskProgress() const
{
    countTasks(m_players, m_tasksDone, m_tasksTotal);
    if (m_tasksTotal == 0) {
        return 0.0;
    }
    return static_cast<double>(m_tasksDone) / m_tasksTotal;
}

Winner GameState::checkWinCondition() const
{
    if (m_phase == GamePhase::Lobby) {
        return Winner::None;
    }

    int impostors = 0;
    int crew = 0;
    for (const Player& p : m_players) {
        if (!p.isAlive()) {
            continue;
        }
        if (p.isImpostor()) {
            ++impostors;
        } else {
            ++crew;
        }
    }

    // 1) Nema zivih impostora -> posada pobedjuje.
    if (impostors == 0) {
        return Winner::Crewmates;
    }
    // 2) Impostora koliko i posade (ili vise) -> impostori pobedjuju.
    if (impostors >= crew) {
        return Winner::Impostors;
    }
    // 3) Posada je zavrsila sve taskove.
    countTasks(m_players, m_tasksDone, m_tasksTotal);
    if (m_tasksTotal > 0 && m_tasksDone >= m_tasksTotal) {
        return Winner::Crewmates;
    }
    return Winner::None;
}

// ---------------------------------------------------------------- JSON

QJsonObject GameState::toJson() const
{
    countTasks(m_players, m_tasksDone, m_tasksTotal);

    QJsonObject obj;
    obj["phase"]      = static_cast<int>(m_phase);
    obj["tasksDone"]  = m_tasksDone;
    obj["tasksTotal"] = m_tasksTotal;

    QJsonArray players;
    for (const Player& p : m_players) {
        players.append(playerToJson(p));
    }
    obj["players"] = players;

    QJsonArray bodies;
    for (const DeadBody& b : m_bodies) {
        QJsonObject bj;
        bj["victimId"] = b.victimId();
        bj["x"] = b.pos().x();
        bj["y"] = b.pos().y();
        bodies.append(bj);
    }
    obj["bodies"] = bodies;
    return obj;
}

void GameState::fromJson(const QJsonObject& obj)
{
    m_phase      = static_cast<GamePhase>(obj.value("phase").toInt());
    m_tasksDone  = obj.value("tasksDone").toInt();
    m_tasksTotal = obj.value("tasksTotal").toInt();

    // Svi postojeci taskovi po ID-u. Postojeci se AZURIRAJU, a ne prave
    // ponovo, da bi Task* u otvorenom TaskDialog-u ostao ispravan.
    QMap<int, Task*> oldTasks;
    for (const Player& p : m_players) {
        for (Task* t : p.tasks()) {
            if (t != nullptr) {
                oldTasks.insert(t->id(), t);
            }
        }
    }

    QSet<int> seenPlayers;
    QSet<int> seenTasks;

    for (const QJsonValue& v : obj.value("players").toArray()) {
        const QJsonObject pj = v.toObject();
        const int id = pj.value("id").toInt(-1);
        if (id < 0) {
            continue;
        }
        seenPlayers.insert(id);

        Player& p = m_players[id];      // pravi novog ako ga nema
        if (p.id() != id) {
            p = Player(id, QString());
        }
        p.setName(pj.value("name").toString());
        p.setColor(QColor(pj.value("color").toString()));
        p.setPos(QPointF(pj.value("x").toDouble(), pj.value("y").toDouble()));
        p.setRole(static_cast<Role>(pj.value("role").toInt()));
        p.setAlive(pj.value("alive").toBool(true));
        p.setKillCooldown(pj.value("killCooldown").toDouble());

        QVector<Task*> tasks;
        for (const QJsonValue& tv : pj.value("tasks").toArray()) {
            const QJsonObject tj = tv.toObject();
            const int taskId = tj.value("id").toInt(-1);

            Task* t = oldTasks.value(taskId, nullptr);
            if (t == nullptr) {
                t = createTask(tj.value("type").toString(), taskId,
                               tj.value("name").toString(),
                               QPointF(tj.value("x").toDouble(),
                                       tj.value("y").toDouble()));
                if (t == nullptr) {
                    continue;           // nepoznata vrsta taska
                }
                oldTasks.insert(taskId, t);
            }
            if (tj.value("completed").toBool() && !t->isCompleted()) {
                t->markCompleted();     // server je potvrdio
            }
            tasks.append(t);
            seenTasks.insert(taskId);
        }
        p.setTasks(tasks);
    }

    // Igraci kojih vise nema (izasli iz igre).
    for (auto it = m_players.begin(); it != m_players.end();) {
        if (seenPlayers.contains(it.key())) {
            ++it;
        } else {
            it = m_players.erase(it);
        }
    }

    // Taskovi kojih vise nema u snapshotu.
    for (auto it = oldTasks.cbegin(); it != oldTasks.cend(); ++it) {
        if (!seenTasks.contains(it.key())) {
            it.value()->deleteLater();
        }
    }

    m_bodies.clear();
    for (const QJsonValue& v : obj.value("bodies").toArray()) {
        const QJsonObject bj = v.toObject();
        m_bodies.append(DeadBody(bj.value("victimId").toInt(-1),
                                 QPointF(bj.value("x").toDouble(),
                                         bj.value("y").toDouble())));
    }
}
