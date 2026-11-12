#ifndef PLAYERITEM_H
#define PLAYERITEM_H

#include <QObject>
#include <QGraphicsItem>
#include <QPixmap>
#include <QTimer>
#include <QString>
#include <QPointF>

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

class PlayerItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    explicit PlayerItem(QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;

    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

    void setName(const QString& name);
    void setSkinId(int skinId);
    void setAlive(bool alive);
    void setMoving(bool moving);
    void setDirection(const QPointF& direction);

    int skinId() const;
    bool isAlive() const;
    bool isMoving() const;

private:
    void loadSkin();
    void updateSprite();

    QString m_name;

    int m_skinId = 1;

    QPixmap m_idleSprite;
    QPixmap m_walkSprites;
    QPixmap m_deadSprite;
    QPixmap m_ghostSprite;
    QPixmap m_currentSprite;

    bool m_alive = true;
    bool m_moving = false;
    bool m_facingRight = true;

    int m_frameIndex = 0;

    QTimer m_animationTimer;
};

#endif // PLAYERITEM_H
