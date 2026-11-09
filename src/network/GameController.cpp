#include "network/GameController.h"

#include "game/GameState.h"
#include "game/Player.h"
#include "network/Server.h"

#include <QJsonArray>
#include <QJsonObject>

#include "game/VotingSession.h"

#include <QRandomGenerator>
#include <QSet>

#include "game/GameMap.h"
#include "game/tasks/WireTask.h"


#include<iostream>

GameController::GameController(Server* server,
                               GameState* state,
                               QObject* parent)
: QObject(parent),
m_server(server),
m_state(state),
m_voting(nullptr)
{
    m_emergencyCooldown = 0.0;
    m_tickTimer.setInterval(20);
    m_snapshotTimer.setInterval(50);
    m_stateSnapshotTimer.setInterval(200);
    m_meetingTimer.setSingleShot(true);

    connect(
    m_server,
    &Server::messageReceived, this,
    &GameController::onMessage
    );

    connect(
    m_server,
    &Server::playerDisconnected, this,
    &GameController::onPlayerLeft
    );
    connect(
      &m_tickTimer,&QTimer::timeout,
      this, &GameController::tick
  );

    connect(
        &m_snapshotTimer, &QTimer::timeout,
        this, &GameController::sendPositions
    );

    connect(
    &m_stateSnapshotTimer,
    &QTimer::timeout,
    this,
    [this]() {
        if (m_state->phase() == GamePhase::Playing) {
            sendSnapshot();
        }
    }
);

    connect(
    &m_meetingTimer, &QTimer::timeout,
    this, &GameController::endMeeting
);

    // Kada tajmer istekne, timeout aktivira ovu logiku.
    // Posle diskusije prelazimo na glasanje i ponovo pokrecemo tajmer.
    // Posle isteka vremena za glasanje pozivamo endMeeting().
    connect(
    &m_meetingTimer, &QTimer::timeout,
    this, &GameController::onMeetingTimeout
);
}


GameController::~GameController()
{
    delete m_voting;
}

void GameController::startGame()
{
    if (m_state->players().size() < 4 ||
        m_state->phase() != GamePhase::Lobby) {
        return;
        }

    assignRoles();
    assignTasks();


    const auto players = m_state->players();

    for (const auto& copy : players) {
        Player* player = m_state->player(copy.id());

        player->setInputDir(QPointF(0, 0));
        player->setAlive(true);
        player->setKillCooldown(
            player->isImpostor() ? KILL_COOLDOWN : 0.0
        );
    }
    m_state->setPhase(GamePhase::Playing);
    m_emergencyCooldown = EMERGENCY_COOLDOWN;

    QJsonObject publicState = m_state->toJson();
    QJsonArray publicPlayers = publicState["players"].toArray();

    for (int i = 0; i < publicPlayers.size(); ++i)
    {
        QJsonObject player = publicPlayers[i].toObject();

        player.remove("role");
        player.remove("tasks");
        player.remove("killCooldown");

        publicPlayers[i] = player;
    }

    publicState["players"] = publicPlayers;

    m_server->broadcast(
        Message(MessageType::StartGame, publicState));

    sendSnapshot();

    m_tickTimer.start();
    m_snapshotTimer.start();
    m_stateSnapshotTimer.start();

};

void GameController::assignRoles()
{
    const auto players = m_state->players();
    const int playerCount = static_cast<int>(players.size());

    if (playerCount < 4) {
        return;
    }
    // u zavisnosti od broja igraca biramo koliko impostora imamo
    const int impostorCount =
        playerCount >= 10 ? 3 :
        playerCount >= 7  ? 2 : 1;

    QSet<int> impostorIndices;

    while (impostorIndices.size() < impostorCount) {
        const int index =
            QRandomGenerator::global()->bounded(playerCount);

        impostorIndices.insert(index);
    }

    for (int i = 0; i < playerCount; ++i) {
        Player* player = m_state->player(players[i].id());

        player->setRole(
            impostorIndices.contains(i)
                ? Role::Impostor
                : Role::Crewmate
        );
    }
}
void GameController::handleStartGame( int playerId, const Message& msg)
{
    if (!m_server->isHost(playerId) ||
        m_state->player(playerId) == nullptr) {
        return;
        }

    startGame();
}


void GameController::onMessage(int playerId, const Message& msg)
{
    switch (msg.type())
    {
    case MessageType::Join:
        handleJoin(playerId, msg);
        break;
    case MessageType::MoveInput:
        handleMove(playerId, msg);
        break;
    case MessageType::KillRequest:
        handleKill(playerId, msg);
        break;
    case MessageType::ReportBody:
        handleReport(playerId, msg);
        break;
    case MessageType::TaskCompleted:
        handleTaskCompleted(playerId, msg);
        break;
    case MessageType::Vote:
        handleVote(playerId, msg);
        break;
    case MessageType::ChatMessage:
        handleChat(playerId, msg);
        break;
        case MessageType::StartGame:
        handleStartGame(playerId, msg);
        break;
    case MessageType::EmergencyMeeting:
        //on ce prvo pozvati canCallEmergency
        handleEmergency(playerId, msg);
        break;
        case MessageType::ChooseSkin:
            handleChooseSkin(playerId, msg);
            break;
    default:
        break;

    }
}

//mora da se dopuni kada ubacimo jos taskova
// trenutna funkcija smao dodeljuje svim crewmate-ima po 3 taska tipa zice
// ali na razlicitim mestima i tjt

void GameController::assignTasks()
{
    static GameMap* map = m_state->getMap();

    if (map == nullptr) {
        return;
    }

    const auto stations = map->taskStations();

    if (stations.isEmpty()) {
        return;
    }

    const int stationCount = static_cast<int>(stations.size());
    const int taskCount = qMin(3, stationCount);

    int nextTaskId = 1;

    const auto players = m_state->players();

    for (const auto& copy : players) {
        Player* player = m_state->player(copy.id());

        if (player->isImpostor()) {
            continue;
        }

        QSet<int> selectedStations;

        while (selectedStations.size() < taskCount) {
            const int index =
                QRandomGenerator::global()->bounded(stationCount);

            // Ovaj igrac vec ima task na toj stanici.
            if (selectedStations.contains(index)) {
                continue;
            }

            selectedStations.insert(index);

            auto* task = new WireTask(
                nextTaskId++,
                QStringLiteral("Povezi zice"),
                stations[index]
            );

            player->assignTask(task);
        }
    }
}

void GameController::handleJoin(int playerId, const Message& msg)
{
    bool conditions = m_state->phase() == GamePhase::Lobby &&
        m_state->players().size() < 16 &&
        m_state->player(playerId) == nullptr;

    Message incoming = msg;
    QJsonObject playerJson = incoming.get_messageContent();
    QString name = playerJson.value("name").toString().trimmed();

    for (const auto& copy : m_state->players())
    {
        if (copy.name() == name)
            conditions = false;
    }
    //proverimo jos da li je ime prazno i da li je dozvoljene duzine
    conditions = conditions && !name.isEmpty() && name.size() <= 20;

    if (!conditions)
    {
        return;
    }

    Player player(playerId, name);

    m_state->addPlayer(player);

    m_server->sendTo(playerId, Message(MessageType::JoinAccepted, QJsonObject{}));

    sendLobbyUpdate();
}

void GameController::handleMove(int playerId, const Message& msg)
{
    if (m_state->phase() != GamePhase::Playing) {
        return;
    }

    Player* player = m_state->player(playerId);

    if (player == nullptr || !player->isAlive()) {
        return;
    }

    Message incoming = msg;
    const QJsonObject content = incoming.get_messageContent();

    if (!content.value("dx").isDouble() ||
        !content.value("dy").isDouble()) {
        return;
        }

    const double dx = content.value("dx").toDouble();
    const double dy = content.value("dy").toDouble();

    if (!std::isfinite(dx) || !std::isfinite(dy)) {
        return;
    }

    player->setInputDir(QPointF(dx, dy));
}

void GameController::handleChooseSkin(int playerId, const Message& msg)
{
    if (m_state->phase() != GamePhase::Lobby) {
        return;
    }

    Player* player = m_state->player(playerId);
    if (player == nullptr) {
        return;
    }

    Message incoming = msg;
    const QJsonObject content = incoming.get_messageContent();

    if (!content.value("skinId").isDouble()) {
        return;
    }

    const int skinId = content.value("skinId").toInt(1);
    if (skinId < 1 || skinId > SKIN_COUNT) {
        return;
    }

    player->setSkinId(skinId);
    sendLobbyUpdate();
}

void GameController::tick()
{
    if (!m_tickClock.isValid()) {
        m_tickClock.start();
        return;
    }

    // restart() Vrati koliko je milisekundi prošlo od prethodnog početka merenja.
    //Ponovo započne merenje od nule za sledeći tick().
    // Meri stvarno proteklo vreme izmedju poziva tick().
    // dt je to vreme u sekundama; koristi se za pomeranje i cooldownove,
    // kako bi brzina igre ostala ista i kada poziv tajmera zakasni.
    const double dt = m_tickClock.restart() / 1000.0;

    if (m_state->phase() != GamePhase::Playing) {
        return;
    }

    GameMap* map = m_state->getMap();

    if (map == nullptr) {
        return;
    }

    // Za sada pretpostavljamo precnik igraca od 40 px.
    constexpr double playerRadius = 20.0;

    m_emergencyCooldown =
        qMax(0.0, m_emergencyCooldown - dt);

    const auto players = m_state->players();

    for (const auto& copy : players) {
        Player* player = m_state->player(copy.id());

        // ovo je tick za killCooldown
        player->tickCooldown(dt);

        if (!player->isAlive()) {
            continue;
        }

        const QPointF nextPosition =
            player->pos() +
            player->inputDir() * MOVE_SPEED * dt;

        if (map->isWalkable(nextPosition, playerRadius)) {
            player->setPos(nextPosition);
        }
    }
}
void GameController::handleKill(int playerId, const Message& msg)
{
    if (m_state->phase() != GamePhase::Playing) {
        return;
    }

    Player* attacker = m_state->player(playerId);

    if (attacker == nullptr ||
        !attacker->isAlive() ||
        !attacker->isImpostor() ||
        attacker->killCooldown() > 0.0)
    {
        return;
    }

    Message incoming = msg;
    const QJsonObject content = incoming.get_messageContent();
    const int targetId = content.value("targetId").toInt(-1);

    Player* killer = m_state->player(playerId);
    Player* target = m_state->player(targetId);

    bool canKill = false;

    const double distance = QLineF(killer->pos(), target->pos()).length();

    if (distance <= KILL_RANGE) {
        canKill = true;
    }

    if (target == nullptr ||
        targetId == playerId ||
        !target->isAlive() ||
        target->isImpostor() || !canKill)
    {
        return;
    }

    // Zahtev je prihvacen.
    target->setAlive(false);

    //pravimo deadbody da posle moze da se iscrta
    m_state->addBody(
        DeadBody(target->id(), target->pos())
    );

    attacker->setKillCooldown(KILL_COOLDOWN);

    // Obavestavamo klijente o promeni.
    sendSnapshot();

    // Ubistvo je mozda zavrsilo partiju.
    checkWin();
}

void GameController::handleReport(int playerId, const Message& msg)
{
    if (m_state->phase() != GamePhase::Playing) {
        return;
    }

    Player* player = m_state->player(playerId);

    if (player == nullptr || !player->isAlive()) {
        return;
    }

    Message incoming = msg;
    const QJsonObject content = incoming.get_messageContent();
    const int bodyId = content.value("bodyId").toInt(-1);

    const auto bodies = m_state->bodies();

    for (const DeadBody& body : bodies) {
        if (body.victimId() != bodyId) {
            continue;
        }
        if (QLineF(player->pos(), body.pos()).length() > REPORT_RANGE) {
            return;   // igrac je predaleko od tela
        }
        startMeeting(playerId, bodyId);
        return;
    }
}

void GameController::handleTaskCompleted(
    int playerId,
    const Message& msg
)
{
    if (m_state->phase() != GamePhase::Playing) {
        return;
    }

    Player* player = m_state->player(playerId);

    if (player == nullptr || player->isImpostor()) {
        return;
    }

    Message incoming = msg;
    const QJsonObject content = incoming.get_messageContent();
    const int taskId = content.value("taskId").toInt(-1);

    for (Task* task : player->tasks()) {
        if (task == nullptr || task->id() != taskId) {
            continue;
        }

        if (task->isCompleted()) {
            return;
        }

        task->markCompleted();

        checkWin();

        if (m_state->phase() != GamePhase::GameOver) {
            sendSnapshot();
        }

        return;
    }
}

void GameController::handleVote(int playerId, const Message& msg)
{
    if (m_state->phase() != GamePhase::Voting ||
        m_voting == nullptr)
    {
        return;
    }

    Player* player = m_state->player(playerId);

    if (player == nullptr || !player->isAlive()) {
        return;
    }

    Message incoming = msg;
    const QJsonObject content = incoming.get_messageContent();

    // -2 oznacava neispravan ili nedostajuci targetId.
    const int targetId = content.value("targetId").toInt(-2);

    // -1 je skip
    if (targetId != -1) {
        Player* target = m_state->player(targetId);

        if (target == nullptr || !target->isAlive()) {
            return;
        }
    }

    if (!m_voting->castVote(playerId, targetId)) {
        return;
    }

    if (m_voting->allVoted()) {
        endMeeting();
    }
}

void GameController::handleChat(int playerId, const Message& msg)
{
    const GamePhase phase = m_state->phase();

    if ((phase != GamePhase::Discussion &&
         phase != GamePhase::Voting) ||
        m_voting == nullptr)
    {
        return;
    }

    Player* player = m_state->player(playerId);

    if (player == nullptr || !player->isAlive()) {
        return;
    }

    Message incoming = msg;
    const QJsonObject content = incoming.get_messageContent();
    const QString text = content.value("text").toString().trimmed();

    if (text.isEmpty()) {
        return;
    }

    ChatMessage chatMessage(
        playerId,
        player->name(),
        text
    );

    m_voting->addChat(chatMessage);

    // Server salje poruku svim klijentima preko broadcast().
    // Svaki ClientController emituje chatReceived(), povezan sa
    // ChatWidget::appendMessage(), koji prikazuje ime posiljaoca i tekst.
    m_server->broadcast(
        Message(MessageType::ChatMessage, chatMessage.toJson())
    );
}

bool GameController::canCallEmergency( int playerId)
{
    if (m_state->phase() != GamePhase::Playing ||
        m_voting != nullptr ||
        m_emergencyCooldown > 0.0)
    {
        return false;
    }

    const Player* player = m_state->player(playerId);

    return player != nullptr && player->isAlive();
}

void GameController::handleEmergency(int playerId, const Message& msg)
{
    if (!canCallEmergency(playerId)) {
        return;
    }

    m_emergencyCooldown = EMERGENCY_COOLDOWN;

    startMeeting(playerId, -1);
}

void GameController::startMeeting(int callerId, int bodyId)
{
    if (m_state->phase() != GamePhase::Playing ||
        m_voting != nullptr)
    {
        return;
    }

    QSet<int> eligible;
    const auto players = m_state->players();

    for (const auto& copy : players) {
        Player* player = m_state->player(copy.id());

        // da se igrac nakon zavrsetka meeting-a ne bi kretao dok
        // ne dodje novi podatak o pritisnutom taster
        player->setInputDir(QPointF(0, 0));

        if (player->isAlive()) {
            eligible.insert(player->id());
        }
    }

    m_voting = new VotingSession(
        callerId,
        bodyId,
        eligible
    );

    m_state->setPhase(GamePhase::Discussion);
    m_stateSnapshotTimer.stop();

    sendPhaseChanged();
    sendSnapshot();

    m_meetingTimer.start(
        static_cast<int>(DISCUSSION_TIME * 1000)
    );
}


// Isti slot reaguje na oba isteka:
// Posle diskusije vidi fazu Discussion, prebaci na Voting i ponovo pokrene tajmer sa VOTING_TIME.
// Posle glasanja vidi fazu Voting i pozove endMeeting().
void GameController::onMeetingTimeout()
{
    if (m_voting == nullptr) {
        return;
    }

    if (m_state->phase() == GamePhase::Discussion) {
        m_state->setPhase(GamePhase::Voting);

        sendPhaseChanged();
        sendSnapshot();

        m_meetingTimer.start(
            static_cast<int>(VOTING_TIME * 1000)
        );
    }
    else if (m_state->phase() == GamePhase::Voting) {
        endMeeting();
    }
}

void GameController::endMeeting()
{
    if (m_state->phase() != GamePhase::Voting ||
        m_voting == nullptr)
    {
        return;
    }

    m_meetingTimer.stop();

    // -1 znaci da niko nije izbacen.
    int ejectedId = m_voting->tally();
    bool wasImpostor = false;

    if (ejectedId != -1) {
        Player* player = m_state->player(ejectedId);

        if (player != nullptr && player->isAlive()) {
            wasImpostor = player->isImpostor();
            player->setAlive(false);
        }
        else {
            // Izabrani igrac je u medjuvremenu napustio partiju.
            ejectedId = -1;
        }
    }

    // Posle sastanka uklanjamo prijavljena i ostala tela.
    m_state->clearBodies();

    delete m_voting;
    m_voting = nullptr;

    m_server->broadcast(
        Message(
            MessageType::VoteResult,
            QJsonObject{
                {"ejectedId", ejectedId},
                {"wasImpostor", wasImpostor}
            }
        )
    );

    checkWin();

    if (m_state->phase() == GamePhase::GameOver) {
        return;
    }

    // Partija se nastavlja.
    m_emergencyCooldown = EMERGENCY_COOLDOWN;
    m_state->setPhase(GamePhase::Playing);

    sendPhaseChanged();
    sendSnapshot();
    m_stateSnapshotTimer.start();
}

void GameController::checkWin()
{
    if (m_state->phase() == GamePhase::Lobby ||
        m_state->phase() == GamePhase::GameOver)
    {
        return;
    }

    const Winner winner = m_state->checkWinCondition();

    if (winner == Winner::None) {
        return;
    }

    m_state->setPhase(GamePhase::GameOver);

    // Zaustavljamo tajmere jer je partija zavrsena, da vise ne aktiviraju
    // slotove za simulaciju, slanje pozicija i istek vremena sastanka.
    m_tickTimer.stop();
    m_snapshotTimer.stop();
    m_meetingTimer.stop();
    m_stateSnapshotTimer.stop();

    // Ako se partija zavrsila tokom sastanka.
    // cak i da nismo nikad dosli do meeting-a gde bismo napravili
    //m_voting = new... svakako mzemo da obrisemo
    // delete nullptr je dozvoljeno
    delete m_voting;
    m_voting = nullptr;

    QJsonObject content = m_state->toJson();
    content["winner"] = static_cast<int>(winner);

    m_server->broadcast(
        Message(MessageType::GameOver, content)
    );
}

void GameController::sendSnapshot()
{
    const QJsonObject fullState = m_state->toJson();
    const QJsonArray allPlayers = fullState["players"].toArray();
    const auto players = m_state->players();

    const bool isPlaying =
        m_state->phase() == GamePhase::Playing;

    for (const Player& receiver : players) {
        QJsonObject content = fullState;
        QJsonArray visiblePlayers;

        // Pripremamo podatke koje ovaj primalac sme da vidi.
        // prvo gledamo svakom igracu spakujemo spisak svih igraca i njihovih rolova ako su
        // istom timu onaj kome saljemo i onaj kog obradjujemo
        for (const QJsonValue& value : allPlayers) {
            QJsonObject playerJson = value.toObject();
            const int id = playerJson["id"].toInt(-1);

            if (id != receiver.id()) {
                // Impostori mogu da znaju ko su im saigraci.
                const bool teammate =
                    receiver.isImpostor() &&
                    playerJson["role"].toInt() ==
                        static_cast<int>(Role::Impostor);

                if (!teammate) {
                    playerJson.remove("role");
                }

                playerJson.remove("tasks");
                playerJson.remove("killCooldown");
            }

            visiblePlayers.append(playerJson);
        }

        content["players"] = visiblePlayers;

        // Najblizi nezavrseni task ovog igraca.
        // posle ui koristi da bi znao da li da aktivira dugme ili ne
        int nearTaskId = -1;
        double nearestDistance = TASK_RANGE;

        if (isPlaying && !receiver.isImpostor()) {
            for (const Task* task : receiver.tasks()) {
                if (task == nullptr || task->isCompleted()) {
                    continue;
                }

                const double distance =
                    QLineF(
                        receiver.pos(),
                        task->stationPos()
                    ).length();

                if (distance <= nearestDistance) {
                    nearestDistance = distance;
                    nearTaskId = task->id();
                }
            }
        }

        // Da li trenutno postoji meta u dometu za ubistvo.
        //Ako zadržiš canKill u snapshotu, UI pročita tu vrednost:
        // - true → omogući Kill dugme.
        // - false → onemogući ga
        // GameController::sendSnapshot()
        int killTargetId = -1;
        double best = KILL_RANGE;

        if (isPlaying && receiver.isAlive() && receiver.isImpostor()
            && receiver.killCooldown() <= 0.0)
        {
            for (const Player& target : players) {
                if (target.id() == receiver.id() || !target.isAlive() || target.isImpostor())
                    continue;

                const double d = QLineF(receiver.pos(), target.pos()).length();
                if (d <= best) {
                    best = d;
                    killTargetId = target.id();   // najbliža meta, ne prva
                }
            }
        }

        int nearBodyId = -1;
        double bestBody = REPORT_RANGE;

        if (isPlaying && receiver.isAlive()) {
            for (const DeadBody& body : m_state->bodies()) {
                const double d = QLineF(receiver.pos(), body.pos()).length();
                if (d <= bestBody) {
                    bestBody = d;
                    nearBodyId = body.victimId();
                }
            }
        }

        content["nearBodyId"] = nearBodyId;
        content["killTargetId"] = killTargetId;
        content["nearTaskId"] = nearTaskId;
        content["canEmergency"] = canCallEmergency(receiver.id());

        m_server->sendTo(
            receiver.id(),
            Message(MessageType::StateSnapshot, content)
        );
    }
}

//sendPositions() uzima trenutne pozicije iz GameState-a,
//pravi PositionSnapshot i šalje ga svim klijentima preko UDP-a.
void GameController::sendPositions()
{
    if (m_state->phase() != GamePhase::Playing) {
        return;
    }

    QJsonArray positions;

    const auto players = m_state->players();

    for (const Player& player : players) {
        QJsonObject playerJson;

        playerJson["id"] = player.id();
        playerJson["x"] = player.pos().x();
        playerJson["y"] = player.pos().y();

        positions.append(playerJson);
    }

    QJsonObject content;
    content["players"] = positions;
    content["tick"] =
        static_cast<double>(++m_positionSnapshotTick);
    content["phaseId"] =
        static_cast<int>(m_server->get_m_phaseId());

    m_server->broadcast(
        Message(MessageType::PositionSnapshot, content)
    );
}
//onPlayerLeft() se poziva preko signala Server::playerDisconnected.
//Server je već uklonio mrežnu sesiju; ovde uklanjaš igrača iz stanja
//igre, ažuriraš glasanje i obavestiš preostale igrače.
void GameController::onPlayerLeft(int playerId)
{
    if (m_state->player(playerId) == nullptr) {
        return;
    }

    m_state->removePlayer(playerId);

    // Ako je sastanak aktivan, uklanjamo igraca i iz glasanja.
    if (m_voting != nullptr) {
        m_voting->removePlayer(playerId);
    }

    if (m_state->phase() == GamePhase::Lobby) {
        sendLobbyUpdate();
        return;
    }

    if (m_state->phase() == GamePhase::GameOver) {
        return;
    }

    // Odlazak moze promeniti odnos impostora i posade.
    checkWin();

    if (m_state->phase() == GamePhase::GameOver) {
        return;
    }

    // Mozda su posle odlaska svi preostali igraci vec glasali.
    if (m_state->phase() == GamePhase::Voting &&
        m_voting != nullptr &&
        m_voting->allVoted())
    {
        endMeeting();
        return;
    }

    sendSnapshot();
}

void GameController::sendLobbyUpdate()
{
    if (m_state->phase() != GamePhase::Lobby) {
        return;
    }

    const QJsonObject content = m_state->toJson();

    m_server->broadcast(
        Message(MessageType::LobbyUpdate, content)
    );
}

void GameController::sendPhaseChanged()
{
    const GamePhase phase = m_state->phase();
    int seconds = 0;

    if (phase == GamePhase::Discussion) {
        seconds = static_cast<int>(DISCUSSION_TIME);
    }
    else if (phase == GamePhase::Voting) {
        seconds = static_cast<int>(VOTING_TIME);
    }

    QJsonObject content;
    content["phase"] = static_cast<int>(phase);
    content["seconds"] = seconds;

    m_server->broadcast(
        Message(MessageType::PhaseChanged, content)
    );
}
