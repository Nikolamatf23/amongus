#ifndef CLIENTCONTROLLER_H
#define CLIENTCONTROLLER_H

#include <QObject>
#include <QString>
#include <QJsonObject>

#include "network/Message.h"

class Client;

// Klijentski "mozak": sedi izmedju Client (mreza) i UI-ja / lokalnog stanja igre.
//  - UI poziva njegove metode (sendMove, requestKill, sendVote...)
//  - on pravi odgovarajuci Message i salje ga preko Client-a
//  - poruke sa servera pretvara u signale koje UI/GameState slusa
// Ne zna nista o transportu (TCP/UDP) - to resava Client.
class ClientController : public QObject
{
    Q_OBJECT

public:
    // Client ne posedujemo, samo ga koristimo (vlasnik je npr. MainWindow).
    explicit ClientController(Client* client, QObject* parent = nullptr);

    // Otvara TCP vezu; Join se salje automatski cim se veza uspostavi.
    void connectToServer(const QString& host,
                         quint16 tcpPort,
                         const QString& playerName);
    void disconnectFromServer();

    bool isConnected() const;
    int myId() const;               // -1 dok ne stigne JoinAccepted

    // ---- akcije igraca (UI ih poziva) ----
    void sendMove(double dx, double dy);   // UDP preko Client-a
    void requestKill(int targetId);
    void reportBody(int bodyId);
    void completeTask(int taskId);
    void sendVote(int targetId);           // -1 = skip
    void sendChat(const QString& text);

signals:
    // ---- veza ----
    void connectedToServer();
    void disconnectedFromServer();
    void joined(int myId);                 // stigao JoinAccepted
    void errorOccurred(const QString& text);

    // ---- dogadjaji iz igre (sadrzaj je JSON koji salje GameController) ----
    void lobbyUpdated(const QJsonObject& content);
    void gameStarted(const QJsonObject& content);
    void stateSnapshotReceived(const QJsonObject& content);
    void positionsReceived(const QJsonObject& content);   // UDP
    void phaseChanged(int phaseId, const QJsonObject& content);
    void chatReceived(int senderId, const QString& text);
    void voteResultReceived(const QJsonObject& content);
    void gameOver(const QJsonObject& content);

private slots:
    void onConnected();                       // TCP gore -> posalji Join
    void onDisconnected();
    void onMessage(const Message& msg);       // sve sto stize od servera
    void onError(const QString& text);

private:
    void sendMessage(MessageType type, const QJsonObject& content = QJsonObject());

    Client* m_client;
    QString m_playerName;
};

#endif // CLIENTCONTROLLER_H
