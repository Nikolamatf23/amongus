#include "ui/mainwindow.h"

#include "ui/ChatWidget.h"
#include "ui/GameOverScreen.h"
#include "ui/GameScreen.h"
#include "ui/LobbyScreen.h"
#include "ui/StartScreen.h"
#include "ui/VotingScreen.h"

#include "game/GameMap.h"
#include "game/GameState.h"
#include "network/Client.h"
#include "network/ClientController.h"
#include "network/GameController.h"
#include "network/Server.h"

#include <QMessageBox>
#include <QStackedWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_stack(new QStackedWidget(this))
    , m_start(new StartScreen(m_stack))
    , m_lobby(new LobbyScreen(m_stack))
    , m_voting(new VotingScreen(m_stack))
    , m_over(new GameOverScreen(m_stack))
{
    setWindowTitle(tr("Among Us"));
    resize(1024, 720);

    // GameScreen se ne pravi ovde: treba mu ClientController, a on postoji
    // tek posle createClientSession(). Tamo se GameScreen pravi i ubacuje u stack.
    m_stack->addWidget(m_start);
    m_stack->addWidget(m_lobby);
    m_stack->addWidget(m_voting);
    m_stack->addWidget(m_over);
    setCentralWidget(m_stack);

    // ---- UI -> MainWindow ----
    connect(m_start, &StartScreen::hostClicked, this, &MainWindow::onHost);
    connect(m_start, &StartScreen::joinClicked, this, &MainWindow::onJoin);
    connect(m_lobby, &LobbyScreen::leaveClicked, this, &MainWindow::backToMenu);
    connect(m_over,  &GameOverScreen::backToMenuClicked, this, &MainWindow::backToMenu);

    // ---- UI -> ClientController ----
    // m_ctrl se pravi iznova za svaku sesiju, zato lambde, a ne direktna veza.
    connect(m_lobby, &LobbyScreen::startClicked, this, [this] {
        if (m_ctrl) m_ctrl->requestStart();
    });
    connect(m_voting, &VotingScreen::voteCast, this, [this](int targetId) {
        if (m_ctrl) m_ctrl->castVote(targetId);
    });
    connect(m_voting->chat(), &ChatWidget::messageSubmitted, this, [this](const QString& text) {
        if (m_ctrl) m_ctrl->sendChat(text);
    });

    showScreen(m_start);
}

MainWindow::~MainWindow()
{
    m_leaving = true;
    destroyClientSession();
    stopHosting();
}

// ------------------------------------------------------------ sesija

void MainWindow::createClientSession()
{
    destroyClientSession();

    m_client = std::make_unique<Client>();
    m_state  = std::make_unique<GameState>();
    m_ctrl   = std::make_unique<ClientController>(m_client.get(), m_state.get());

    // GameScreen zivi tacno koliko i ova sesija (drzi pokazivac na m_ctrl).
    m_game = new GameScreen(m_ctrl.get(), m_stack);
    m_stack->addWidget(m_game);

    connect(m_ctrl.get(), &ClientController::lobbyChanged,   this, &MainWindow::onLobbyChanged);
    connect(m_ctrl.get(), &ClientController::phaseChanged,   this, &MainWindow::onPhaseChanged);
    connect(m_ctrl.get(), &ClientController::gameOver,       this, &MainWindow::onGameOver);
    connect(m_ctrl.get(), &ClientController::connectionLost, this, &MainWindow::onConnectionLost);
    connect(m_ctrl.get(), &ClientController::errorOccurred,  this, &MainWindow::onError);

    connect(m_ctrl.get(), &ClientController::chatReceived,
            m_voting->chat(), &ChatWidget::appendMessage);
    connect(m_ctrl.get(), &ClientController::voteResult,
            m_voting, &VotingScreen::showResult);
}

void MainWindow::destroyClientSession()
{
    // GameScreen prvi, jer cita m_ctrl (resizeEvent, updateCamera...).
    // deleteLater umesto delete: moze da se pozove iz exec() petlje TaskDialog-a.
    if (m_game) {
        m_stack->removeWidget(m_game);
        m_game->deleteLater();
        m_game = nullptr;
    }

    if (m_ctrl) {
        // Prvo odvezujemo signale, da gasenje veze ne pozove nase slotove.
        m_ctrl->disconnect(this);
        m_ctrl->disconnectFromServer();
    }
    m_ctrl.reset();
    m_state.reset();
    m_client.reset();
}

void MainWindow::stopHosting()
{
    // Redosled: kontroler koristi server i stanje, pa ide prvi.
    m_gameCtrl.reset();
    if (m_server) {
        m_server->stop();
    }
    m_server.reset();
    m_serverState.reset();
    m_serverMap.reset();
}

// ------------------------------------------------------------ Start ekran

void MainWindow::onHost(const QString& name)
{
    stopHosting();

    m_serverMap = std::make_unique<GameMap>();
    if (!m_serverMap->load(":/map/background.jpg", ":/map/mask.png")) {
        m_serverMap.reset();
        QMessageBox::warning(this, tr("Greska"), tr("Mapa ne moze da se ucita."));
        return;
    }

    m_serverState = std::make_unique<GameState>(m_serverMap.get());
    m_server      = std::make_unique<Server>();
    m_gameCtrl    = std::make_unique<GameController>(m_server.get(), m_serverState.get());

    if (!m_server->startListening(TCP_PORT, UDP_PORT)) {
        stopHosting();
        QMessageBox::warning(this, tr("Greska"),
                             tr("Server ne moze da se pokrene na portu %1. "
                                "Da li je vec pokrenut?").arg(TCP_PORT));
        return;
    }

    m_start->setBusy(true);
    createClientSession();
    m_lobby->setHost(true);
    m_ctrl->connectToServer(QStringLiteral("127.0.0.1"), TCP_PORT, name);
}

void MainWindow::onJoin(const QString& name, const QString& ip)
{
    stopHosting();

    m_start->setBusy(true);
    createClientSession();
    m_lobby->setHost(false);
    m_ctrl->connectToServer(ip, TCP_PORT, name);
}

// ------------------------------------------------------------ signali kontrolera

void MainWindow::onLobbyChanged()
{
    m_lobby->updatePlayers(m_state->players());
    if (m_stack->currentWidget() == m_start) {
        // Prvi LobbyUpdate znaci da smo uspesno usli.
        m_start->setBusy(false);
        showScreen(m_lobby);
    }
}

void MainWindow::onPhaseChanged(GamePhase ph)
{
    switch (ph) {
    case GamePhase::Lobby:
        showScreen(m_lobby);
        break;

    case GamePhase::Playing:
        if (m_game) {
            showScreen(m_game);
            m_game->setFocus();   // da WASD radi odmah
        }
        break;

    case GamePhase::Discussion:
        // Novi sastanak: cist chat, glasanje jos zakljucano.
        m_voting->chat()->clear();
        m_voting->setup(m_state->players(), m_ctrl->myId(), m_ctrl->phaseSeconds());
        m_voting->setVotingEnabled(false);
        showScreen(m_voting);
        break;

    case GamePhase::Voting:
        // Ako je server preskocio diskusiju, ekran jos nije pripremljen.
        if (m_stack->currentWidget() != m_voting) {
            m_voting->chat()->clear();
        }
        m_voting->setup(m_state->players(), m_ctrl->myId(), m_ctrl->phaseSeconds());
        m_voting->setVotingEnabled(true);
        showScreen(m_voting);
        break;

    case GamePhase::GameOver:
        // Rezultat stize posebnom porukom (gameOver signal).
        break;
    }
}

void MainWindow::onGameOver(Winner w)
{
    m_over->showResult(w, m_state->players());
    showScreen(m_over);
}

// Ova dva slota poziva m_ctrl iz svog signala, pa ga ne smemo obrisati odmah:
// backToMenu() se zakazuje (QueuedConnection) da se izvrsi kad se signal zavrsi.
// m_leaving se postavlja odmah, da greska + disconnect ne otvore dva prozora.

void MainWindow::onConnectionLost()
{
    if (m_leaving) {
        return;
    }
    m_leaving = true;
    QMetaObject::invokeMethod(this, &MainWindow::backToMenu, Qt::QueuedConnection);
    QMessageBox::warning(this, tr("Veza prekinuta"), tr("Veza sa serverom je prekinuta."));
}

void MainWindow::onError(const QString& text)
{
    if (m_leaving) {
        return;
    }
    m_leaving = true;
    QMetaObject::invokeMethod(this, &MainWindow::backToMenu, Qt::QueuedConnection);
    QMessageBox::warning(this, tr("Greska"), tr("Povezivanje nije uspelo:\n%1").arg(text));
}

void MainWindow::backToMenu()
{
    m_leaving = true;
    destroyClientSession();
    stopHosting();
    m_leaving = false;

    m_start->setBusy(false);
    showScreen(m_start);
}

// ------------------------------------------------------------ pomocno

void MainWindow::showScreen(QWidget* w)
{
    m_stack->setCurrentWidget(w);
}