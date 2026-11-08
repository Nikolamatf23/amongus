#ifndef PLAYERITEM_H
#define PLAYERITEM_H

#include <QGraphicsItem>
#include <QString>
#include <QTimer>
#include <QPixmap>
#include <QColor>
#include <QObject>
#include <QPainter>
#include "game/GameTypes.h"

class PlayerItem : public QObject, public QGraphicsItem
{
public:
    PlayerItem();

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    void setName(const QString& name);

    //void setColor(ColorIndex color) kada game tim napravi ColorIndex, ovo odkomentarisi;
    void setColor(const QColor& color); //a ovo obrisi

    void updateSprite();

    void setAlive(bool alive);

    void setMoving(bool moving);
private:
    QString m_name;
    //ColorIndex m_color odkomentarisi;
    QColor m_color; //obrisi
    QPixmap m_sprite;
    bool m_alive = true;
    QTimer m_animationTimer;
    int m_frameIndex = 0;
    bool m_moving = false;
};

#endif //PLAYERITEM_H