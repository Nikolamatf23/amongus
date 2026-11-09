#include "ui/GameScreen.h"

#include "ui/PlayerItem.h"
#include "ui/TaskDialog.h"
#include "ui/EmergencyDialog.h"

#include "network/ClientController.h"

#include "game/GameState.h"
#include "game/Player.h"
#include "game/DeadBody.h"
#include "game/Task.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsEllipseItem>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QPixmap>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QtMath>
#include <QScrollBar>
#include <QDebug>

namespace
{
constexpr qreal BODY_SIZE = 28.0;
constexpr qreal MOVEMENT_EPSILON = 0.1;
constexpr qreal CAMERA_ZOOM = 1.55;
}

GameScreen::GameScreen(ClientController* controller, QWidget* parent)
    : QWidget(parent),
    m_controller(controller)
{
    setupUi();
    setupMap();

    if (m_controller != nullptr) {
        connect(m_controller, &ClientController::stateChanged,
                this, &GameScreen::syncState);

        connect(m_controller, &ClientController::phaseChanged,
                this, [this](GamePhase) {
                    if (m_controller == nullptr ||
                        m_controller->state() == nullptr ||
                        m_controller->state()->phase() != GamePhase::Playing) {
                        stopMovement();
                    }
                    syncState();
                });

        syncState();
    }
}

void GameScreen::setCameraTargetPlayerId(int playerId)
{
    m_cameraTargetOverride = playerId;
    updateCamera();
}

int GameScreen::cameraTargetPlayerId() const
{
    if (m_cameraTargetOverride >= 0) {
        return m_cameraTargetOverride;
    }

    return m_controller != nullptr ? m_controller->myId() : -1;
}

void GameScreen::setupUi()
{
    setFocusPolicy(Qt::StrongFocus);

    m_scene = new QGraphicsScene(this);
    m_view = new QGraphicsView(m_scene, this);

    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setRenderHint(QPainter::SmoothPixmapTransform);
    m_view->setDragMode(QGraphicsView::NoDrag);
    m_view->setFocusPolicy(Qt::NoFocus);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setAlignment(Qt::AlignCenter);
    m_view->setTransformationAnchor(QGraphicsView::NoAnchor);
    m_view->setResizeAnchor(QGraphicsView::NoAnchor);

    m_useButton = new QPushButton(tr("Use"), this);
    m_killButton = new QPushButton(tr("Kill"), this);
    m_reportButton = new QPushButton(tr("Report"), this);
    m_emergencyButton = new QPushButton(tr("Emergency"), this);

    auto* buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(m_useButton);
    buttonsLayout->addWidget(m_killButton);
    buttonsLayout->addWidget(m_reportButton);
    buttonsLayout->addWidget(m_emergencyButton);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_view);
    mainLayout->addLayout(buttonsLayout);

    connect(m_useButton, &QPushButton::clicked,
            this, &GameScreen::onUseClicked);
    connect(m_killButton, &QPushButton::clicked,
            this, &GameScreen::onKillClicked);
    connect(m_reportButton, &QPushButton::clicked,
            this, &GameScreen::onReportClicked);
    connect(m_emergencyButton, &QPushButton::clicked,
            this, &GameScreen::onEmergencyClicked);

    updateButtons();
}

void GameScreen::setupMap()
{
    // Ovaj alias mora da postoji u lokalnom resources.qrc.
    QPixmap mapPixmap(":/map/background.jpg");

    if (mapPixmap.isNull()) {
        qWarning() << "GameScreen: nije moguce ucitati :/map/background.jpg";
        m_scene->setSceneRect(0, 0, 1200, 800);
    } else {
        m_mapItem = m_scene->addPixmap(mapPixmap);
        m_mapItem->setZValue(-100);
        m_scene->setSceneRect(m_mapItem->boundingRect());
    }

    if (!m_zoomInitialized) {
        m_view->scale(CAMERA_ZOOM, CAMERA_ZOOM);
        m_zoomInitialized = true;
    }

    m_view->centerOn(m_scene->sceneRect().center());
}

void GameScreen::syncState()
{
    if (m_controller == nullptr || m_controller->state() == nullptr) {
        return;
    }

    syncPlayers();
    syncBodies();
    updateButtons();
    updateCamera();
}

void GameScreen::syncPlayers()
{
    const GameState* state = m_controller->state();
    const QList<Player> players = state->players();

    QSet<int> currentPlayerIds;

    for (const Player& player : players) {
        const int id = player.id();
        currentPlayerIds.insert(id);

        PlayerItem* item = m_playerItems.value(id, nullptr);

        if (item == nullptr) {
            item = new PlayerItem;
            m_scene->addItem(item);
            item->setZValue(10);
            m_playerItems.insert(id, item);
        }

        item->setName(player.name());
        item->setSkinId(player.skinId());
        item->setAlive(player.isAlive());

        const QPointF currentPosition = player.pos();

        if (m_previousPositions.contains(id)) {
            const QPointF difference =
                currentPosition - m_previousPositions.value(id);

            const qreal distance = qSqrt(
                difference.x() * difference.x() +
                difference.y() * difference.y());

            const bool moving = distance > MOVEMENT_EPSILON;
            item->setMoving(moving);

            if (moving) {
                item->setDirection(difference);
            }
        } else {
            item->setMoving(false);
        }

        item->setPos(currentPosition);
        m_previousPositions.insert(id, currentPosition);
    }

    const QList<int> existingIds = m_playerItems.keys();

    for (int id : existingIds) {
        if (currentPlayerIds.contains(id)) {
            continue;
        }

        PlayerItem* item = m_playerItems.take(id);
        m_scene->removeItem(item);
        delete item;
        m_previousPositions.remove(id);
    }
}

void GameScreen::syncBodies()
{
    const GameState* state = m_controller->state();
    const QVector<DeadBody> bodies = state->bodies();

    QSet<int> currentBodyIds;

    for (const DeadBody& body : bodies) {
        const int id = body.victimId();
        currentBodyIds.insert(id);

        QGraphicsEllipseItem* item = m_bodyItems.value(id, nullptr);

        if (item == nullptr) {
            item = m_scene->addEllipse(
                -BODY_SIZE / 2.0, -BODY_SIZE / 2.0,
                BODY_SIZE, BODY_SIZE,
                QPen(Qt::black), QBrush(Qt::red));
            item->setZValue(5);
            m_bodyItems.insert(id, item);
        }

        item->setPos(body.pos());
    }

    const QList<int> existingIds = m_bodyItems.keys();

    for (int id : existingIds) {
        if (currentBodyIds.contains(id)) {
            continue;
        }

        QGraphicsEllipseItem* item = m_bodyItems.take(id);
        m_scene->removeItem(item);
        delete item;
    }
}

void GameScreen::updateCamera()
{
    if (m_view == nullptr || m_scene == nullptr) {
        return;
    }

    const int targetId = cameraTargetPlayerId();
    if (targetId < 0 || m_controller == nullptr ||
        m_controller->state() == nullptr) {
        return;
    }

    const Player* target = m_controller->state()->player(targetId);
    if (target == nullptr) {
        return;
    }

    const QRectF bounds = m_scene->sceneRect();
    const qreal sx = qAbs(m_view->transform().m11());
    const qreal sy = qAbs(m_view->transform().m22());

    if (sx <= 0.0 || sy <= 0.0) {
        m_view->centerOn(target->pos());
        return;
    }

    // Veličina vidljivog dela scene u koordinatama mape.
    const qreal halfWidth = m_view->viewport()->width() / (2.0 * sx);
    const qreal halfHeight = m_view->viewport()->height() / (2.0 * sy);

    QPointF center = target->pos();

    // Uz ivice mape kamera prestaje da prati centar igrača kako
    // ne bi prikazivala prostor izvan same mape.
    if (bounds.width() <= 2.0 * halfWidth) {
        center.setX(bounds.center().x());
    } else {
        center.setX(qBound(bounds.left() + halfWidth,
                           center.x(), bounds.right() - halfWidth));
    }

    if (bounds.height() <= 2.0 * halfHeight) {
        center.setY(bounds.center().y());
    } else {
        center.setY(qBound(bounds.top() + halfHeight,
                           center.y(), bounds.bottom() - halfHeight));
    }

    m_view->centerOn(center);
}

void GameScreen::updateButtons()
{
    const bool connected =
        m_controller != nullptr && m_controller->isConnected();

    m_useButton->setEnabled(
        connected && m_controller->nearTaskId() >= 0);
    m_killButton->setEnabled(
        connected && m_controller->canKill());
    m_reportButton->setEnabled(
        connected && m_controller->canReport());
    m_emergencyButton->setEnabled(
        connected && m_controller->canCallEmergency());
}

void GameScreen::keyPressEvent(QKeyEvent* event)
{
    const int key = event->key();

    if (key == Qt::Key_W || key == Qt::Key_A ||
        key == Qt::Key_S || key == Qt::Key_D ||
        key == Qt::Key_Up || key == Qt::Key_Down ||
        key == Qt::Key_Left || key == Qt::Key_Right) {

        if (!event->isAutoRepeat()) {
            m_pressedKeys.insert(key);
            updateMovement();
        }

        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

void GameScreen::keyReleaseEvent(QKeyEvent* event)
{
    const int key = event->key();

    if (key == Qt::Key_W || key == Qt::Key_A ||
        key == Qt::Key_S || key == Qt::Key_D ||
        key == Qt::Key_Up || key == Qt::Key_Down ||
        key == Qt::Key_Left || key == Qt::Key_Right) {

        if (!event->isAutoRepeat()) {
            m_pressedKeys.remove(key);
            updateMovement();
        }

        event->accept();
        return;
    }

    QWidget::keyReleaseEvent(event);
}

void GameScreen::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateCamera();
}

void GameScreen::updateMovement()
{
    if (m_controller == nullptr || !m_controller->isConnected()) {
        return;
    }

    const GameState* state = m_controller->state();

    if (state == nullptr || state->phase() != GamePhase::Playing) {
        stopMovement();
        return;
    }

    const Player* player = state->player(m_controller->myId());

    if (player == nullptr || !player->isAlive()) {
        stopMovement();
        return;
    }

    qreal dx = 0.0;
    qreal dy = 0.0;

    if (m_pressedKeys.contains(Qt::Key_W) ||
        m_pressedKeys.contains(Qt::Key_Up)) {
        dy -= 1.0;
    }
    if (m_pressedKeys.contains(Qt::Key_S) ||
        m_pressedKeys.contains(Qt::Key_Down)) {
        dy += 1.0;
    }
    if (m_pressedKeys.contains(Qt::Key_A) ||
        m_pressedKeys.contains(Qt::Key_Left)) {
        dx -= 1.0;
    }
    if (m_pressedKeys.contains(Qt::Key_D) ||
        m_pressedKeys.contains(Qt::Key_Right)) {
        dx += 1.0;
    }

    QPointF direction(dx, dy);
    const qreal length = qSqrt(dx * dx + dy * dy);

    if (length > 0.0) {
        direction /= length;
    }

    if (m_hasSentDirection && direction == m_lastSentDirection) {
        return;
    }

    m_controller->sendMoveInput(direction);
    m_lastSentDirection = direction;
    m_hasSentDirection = true;
}

void GameScreen::stopMovement()
{
    m_pressedKeys.clear();

    if (m_controller != nullptr && m_controller->isConnected()) {
        m_controller->sendMoveInput(QPointF(0.0, 0.0));
    }

    m_lastSentDirection = QPointF(0.0, 0.0);
    m_hasSentDirection = true;
}

void GameScreen::onUseClicked()
{
    if (m_controller == nullptr ||
        m_controller->nearTaskId() < 0 ||
        m_controller->state() == nullptr) {
        return;
    }

    const Player* player =
        m_controller->state()->player(m_controller->myId());

    if (player == nullptr) {
        return;
    }

    Task* task = player->task(m_controller->nearTaskId());

    if (task == nullptr || task->isCompleted()) {
        return;
    }

    TaskDialog dialog(task, m_controller, this);
    dialog.exec();
}

void GameScreen::onKillClicked()
{
    if (m_controller != nullptr && m_controller->canKill()) {
        m_controller->requestKill();
    }
}

void GameScreen::onReportClicked()
{
    if (m_controller != nullptr && m_controller->canReport()) {
        m_controller->reportBody();
    }
}

void GameScreen::onEmergencyClicked()
{
    if (m_controller == nullptr ||
        !m_controller->canCallEmergency()) {
        return;
    }

    EmergencyDialog dialog(this);

    connect(&dialog, &EmergencyDialog::meetingRequested,
            this, [this]() {
                if (m_controller != nullptr &&
                    m_controller->canCallEmergency()) {
                    m_controller->requestEmergency();
                }
            });

    dialog.exec();
}
