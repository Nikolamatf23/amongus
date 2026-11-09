#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// ============================================================================
//  MainWindow
//  Drzi sve ekrane u QStackedWidget-u i prebacuje ih na promenu faze.
//  Vlasnik je klijentske sesije (Client, GameState, ClientController),
//  a kod hosta i serverske strane (Server, GameState, GameController).
//
//  Dijagram:
//    - m_stack: QStackedWidget*
//    - m_start, m_lobby, m_game, m_voting, m_over (ekrani)
//    - m_ctrl: ClientController*
//    - m_server: Server*            (samo host)
//    - m_gameCtrl: GameController*  (samo host)
//    + onHost()
//    + onJoin(name, ip)
//    + onPhaseChanged(ph)
//    + showScreen(w: QWidget*)
// ============================================================================

#include "game/GameTypes.h"

#include <QMainWindow>
#include <memory>

class QStackedWidget;

class StartScreen;
class LobbyScreen;
class GameScreen;
class VotingScreen;
class GameOverScreen;

class Client;
class ClientController;
class GameController;
class GameMap;
class GameState;
class Server;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    static constexpr quint16 TCP_PORT = 45454;
    static constexpr quint16 UDP_PORT = 45455;

private slots:
    void onHost(const QString& name);
    void onJoin(const QString& name, const QString& ip);
    void onPhaseChanged(GamePhase ph);
    void showScreen(QWidget* w);

    void onLobbyChanged();
    void onGameOver(Winner w);
    void onConnectionLost();
    void onError(const QString& text);
    void backToMenu();

private:
    // Pravi novu klijentsku sesiju i povezuje signale kontrolera sa ekranima.
    void createClientSession();
    void destroyClientSession();
    void stopHosting();

    // ---- ekrani ----
    QStackedWidget* m_stack;
    StartScreen*    m_start;
    LobbyScreen*    m_lobby;
    VotingScreen*   m_voting;
    GameOverScreen* m_over;

    // GameScreen drzi pokazivac na ClientController, pa se pravi i brise
    // zajedno sa klijentskom sesijom (createClientSession / destroyClientSession).
    GameScreen*     m_game = nullptr;

    // ---- klijent (svaki igrac) ----
    std::unique_ptr<Client>           m_client;
    std::unique_ptr<GameState>        m_state;
    std::unique_ptr<ClientController> m_ctrl;

    // ---- server (samo host) ----
    std::unique_ptr<GameMap>        m_serverMap;
    std::unique_ptr<GameState>      m_serverState;
    std::unique_ptr<Server>         m_server;
    std::unique_ptr<GameController> m_gameCtrl;

    // true dok namerno prekidamo vezu, da ne prikazemo "veza izgubljena".
    bool m_leaving = false;
};

#endif // MAINWINDOW_H