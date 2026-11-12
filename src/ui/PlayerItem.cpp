#include "ui/PlayerItem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDebug>

namespace
{
constexpr int SPRITE_SIZE = 50;
constexpr int ANIMATION_INTERVAL = 150;
constexpr int FRAME_COUNT = 4;
constexpr int MIN_SKIN_ID = 1;
constexpr int MAX_SKIN_ID = 16;
}

PlayerItem::PlayerItem(QGraphicsItem* parent)
    : QObject(),
    QGraphicsItem(parent)
{
    m_animationTimer.setInterval(ANIMATION_INTERVAL);

    connect(&m_animationTimer, &QTimer::timeout,
            this, [this]()
            {
                if (!m_alive || !m_moving) {
                    return;
                }

                m_frameIndex = (m_frameIndex + 1) % FRAME_COUNT;
                updateSprite();
            });

    loadSkin();
    updateSprite();
}

QRectF PlayerItem::boundingRect() const
{
    // Ime je iznad sprite-a, a sam sprite zauzima 50x50 piksela.
    return QRectF(-70, -48, 140, 75);
}

void PlayerItem::paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // Crtanje sprite-a.
    painter->save();

    if (!m_facingRight) {
        painter->scale(-1.0, 1.0);
    }

    painter->drawPixmap(
        QRectF(-SPRITE_SIZE / 2.0,
               -SPRITE_SIZE / 2.0,
               SPRITE_SIZE,
               SPRITE_SIZE),
        m_currentSprite,
        m_currentSprite.rect()
        );

    painter->restore();

    // Ime se ne preslikava kada se igrač okrene ulevo.
    painter->setPen(Qt::white);

    painter->drawText(
        QRectF(-65, -48, 130, 18),
        Qt::AlignCenter,
        m_name
        );
}

void PlayerItem::setName(const QString& name)
{
    if (m_name == name) {
        return;
    }

    m_name = name;
    update();
}

void PlayerItem::setSkinId(int skinId)
{
    // Podržavamo skinove od 1 do 16.
    if (skinId < MIN_SKIN_ID || skinId > MAX_SKIN_ID) {
        skinId = 1;
    }

    if (m_skinId == skinId) {
        return;
    }

    m_skinId = skinId;

    loadSkin();
    updateSprite();
}

void PlayerItem::setAlive(bool alive)
{
    if (m_alive == alive) {
        return;
    }

    m_alive = alive;

    if (!m_alive) {
        m_moving = false;
        m_frameIndex = 0;
        m_animationTimer.stop();
    }

    updateSprite();
}

void PlayerItem::setMoving(bool moving)
{
    if (!m_alive) {
        moving = false;
    }

    if (m_moving == moving) {
        return;
    }

    m_moving = moving;

    if (m_moving) {
        m_frameIndex = 0;
        m_animationTimer.start();
        updateSprite();
    }
    else {
        m_animationTimer.stop();
        m_frameIndex = 0;
        updateSprite();
    }
}

void PlayerItem::setDirection(const QPointF& direction)
{
    if (direction.x() < 0.0) {
        m_facingRight = false;
    }
    else if (direction.x() > 0.0) {
        m_facingRight = true;
    }

    update();
}

int PlayerItem::skinId() const
{
    return m_skinId;
}

bool PlayerItem::isAlive() const
{
    return m_alive;
}

bool PlayerItem::isMoving() const
{
    return m_moving;
}

void PlayerItem::loadSkin()
{
    const QString skinPrefix =
        QString(":/players/skin%1_")
            .arg(m_skinId, 2, 10, QChar('0'));

    m_idleSprite.load(skinPrefix + "idle.png");
    m_walkSprites.load(skinPrefix + "walk.png");
    m_deadSprite.load(skinPrefix + "dead.png");
    m_ghostSprite.load(QStringLiteral(":/players/duh.png"));

    if (m_idleSprite.isNull() ||
        m_walkSprites.isNull() ||
        m_deadSprite.isNull()) {

        qWarning() << "PlayerItem: nije moguce ucitati skin"
                   << m_skinId;

        // Ako resurs nedostaje, pokušavamo da koristimo prvi skin.
        if (m_skinId != 1) {
            m_skinId = 1;
            loadSkin();
        }
    }
}

void PlayerItem::updateSprite()
{
    if (!m_alive) {
        // Duh; ako slika nedostaje, koristimo idle da se nešto vidi.
        m_currentSprite = m_ghostSprite.isNull() ? m_idleSprite : m_ghostSprite;
    }
    else if (!m_moving) {
        m_currentSprite = m_idleSprite;
    }
    else {
        // walk.png je 2x2 spritesheet:
        // 0 1
        // 2 3

        const int x = (m_frameIndex % 2) * SPRITE_SIZE;
        const int y = (m_frameIndex / 2) * SPRITE_SIZE;

        m_currentSprite = m_walkSprites.copy(
            x,
            y,
            SPRITE_SIZE,
            SPRITE_SIZE
            );
    }

    update();
}
