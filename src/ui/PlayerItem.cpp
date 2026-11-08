#include "PlayerItem.h"
#include <QPainter>
#include <QPixmap>

namespace {
QPixmap getPlayerSprites()
{
    static QPixmap playerSprites(":/players/tripulantes.png");
    return playerSprites;
}
}

PlayerItem::PlayerItem()
{
    updateSprite();

    connect(&m_animationTimer, &QTimer::timeout, this, [this]() {
        m_frameIndex = (m_frameIndex + 1) % 4;
        updateSprite();
    });

    m_animationTimer.setInterval(150);
}

QRectF PlayerItem::boundingRect() const
{
    return QRectF(-50, -30, 100, 45);
    //(x, y, width, height)
}

void PlayerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->drawPixmap(-12, -12, 24, 24, m_sprite);
    painter->drawText(QRectF(-50, -30, 100, 15), Qt::AlignCenter, m_name);
}

void PlayerItem::setName(const QString& name)
{
    m_name = name;
    update();
}

/*void PlayerItem::setColor(ColorIndex color)
{
    m_color = color;
    update();
}
*/
void PlayerItem::setColor(const QColor& color)
{
    m_color = color;
    update();
}

void PlayerItem::updateSprite()
{
    int colorIndex = 0; // Privremeno koristimo prvu boju.

    if (!m_alive)
    {
        m_sprite = getPlayerSprites().copy(25, colorIndex * 25, 25, 25);
    }
    else
    {
        QPixmap animationSprites(":/players/tripulante.png");

        int x = (m_frameIndex % 2) * 25;
        int y = (m_frameIndex / 2) * 25;

        m_sprite = animationSprites.copy(x, y, 25, 25);
    }

    update();
}

void PlayerItem::setAlive(bool alive)
{
    if (m_alive == alive)
        return;

    m_alive = alive;

    if (!m_alive)
    {
        m_animationTimer.stop();
        m_moving = false;
        m_frameIndex = 0;
    }

    updateSprite();
}

void PlayerItem::setMoving(bool moving)
{
    if (m_moving == moving)
        return;

    m_moving = moving;

    if (m_moving && m_alive)
    {
        m_animationTimer.start();
    }
    else
    {
        m_animationTimer.stop();
        m_frameIndex = 0;
        updateSprite();
    }
}
