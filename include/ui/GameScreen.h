#ifndef GAMESCREEN_H
#define GAMESCREEN_H

#include <QWidget>
#include <QHash>
#include <QPointF>
#include <QSet>

class ClientController;
class PlayerItem;
class QGraphicsScene;
class QGraphicsView;
class QGraphicsPixmapItem;
class QGraphicsEllipseItem;
class QPushButton;
class QKeyEvent;
class QResizeEvent;

class GameScreen : public QWidget
{
    Q_OBJECT

public:
    explicit GameScreen(ClientController* controller,
                        QWidget* parent = nullptr);

    // Koristi se i za posmatranje/spektatorski prikaz.
    // U normalnoj partiji cilj kamere je controller->myId().
    void setCameraTargetPlayerId(int playerId);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void syncState();

    void onUseClicked();
    void onKillClicked();
    void onReportClicked();
    void onEmergencyClicked();

private:
    void setupUi();
    void setupMap();

    void syncPlayers();
    void syncBodies();
    void updateButtons();
    void updateCamera();

    void updateMovement();
    void stopMovement();

    int cameraTargetPlayerId() const;

    ClientController* m_controller = nullptr;

    QGraphicsScene* m_scene = nullptr;
    QGraphicsView* m_view = nullptr;
    QGraphicsPixmapItem* m_mapItem = nullptr;

    QHash<int, PlayerItem*> m_playerItems;
    QHash<int, QGraphicsPixmapItem*> m_bodyItems;
    QHash<int, QPointF> m_previousPositions;

    QPushButton* m_useButton = nullptr;
    QPushButton* m_killButton = nullptr;
    QPushButton* m_reportButton = nullptr;
    QPushButton* m_emergencyButton = nullptr;

    QSet<int> m_pressedKeys;
    QPointF m_lastSentDirection;

    int m_cameraTargetOverride = -1;

    bool m_hasSentDirection = false;
    bool m_zoomInitialized = false;
    QHash<int, qint64> m_lastMoveTime;
};

#endif // GAMESCREEN_H
