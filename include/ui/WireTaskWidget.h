#ifndef WIRETASKWIDGET_H
#define WIRETASKWIDGET_H

// ============================================================================
//  WireTaskWidget  (nasledjuje TaskWidget)   (UI)
//  Minigra "spajanje zica": levo zice u boji, desno iste boje izmesane.
//  Igrac misem prevuce zicu do iste boje; kad su sve spojene -> solved().
//  Raspored desne strane se mesa pri svakom otvaranju (u konstruktoru).
// ============================================================================

#include "ui/TaskWidget.h"

#include <QColor>
#include <QPointF>
#include <QVector>

class QPainter;

class WireTaskWidget : public TaskWidget
{
    Q_OBJECT

public:
    explicit WireTaskWidget(int wireCount, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QPointF leftEnd(int i) const;
    QPointF rightEnd(int i) const;
    bool    allConnected() const;

    static bool near(const QPointF& a, const QPointF& b);
    static void drawPlug(QPainter& p, const QPointF& c, const QColor& color);
    static void drawWire(QPainter& p, const QPointF& a, const QPointF& b,
                         const QColor& color);

    int             m_wireCount;
    QVector<QColor> m_colors;      // boje zica, leva strana odozgo nadole
    QVector<int>    m_order;       // m_order[i] = boja na i-tom mestu desno
    QVector<bool>   m_connected;   // m_connected[boja] = spojena
    int             m_dragging = -1;
    QPointF         m_mouse;
    bool            m_solved = false;
};

#endif // WIRETASKWIDGET_H
