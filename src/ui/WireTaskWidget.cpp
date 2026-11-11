#include "ui/WireTaskWidget.h"

#include <QLineF>
#include <QMouseEvent>
#include <QPainter>
#include <QRandomGenerator>

#include <algorithm>

namespace
{
// Boje koje minigra moze da koristi (najvise ovoliko zica).
const QVector<QColor>& allWireColors()
{
    static const QVector<QColor> colors = {
        QColor(220, 40, 40),     // crvena
        QColor(40, 90, 230),     // plava
        QColor(240, 220, 40),    // zuta
        QColor(230, 60, 200)     // roze
    };
    return colors;
}
} // namespace

WireTaskWidget::WireTaskWidget(int wireCount, QWidget* parent)
    : TaskWidget(parent)
    , m_wireCount(std::clamp(wireCount, 1,
                             static_cast<int>(allWireColors().size())))
{
    m_colors = allWireColors().mid(0, m_wireCount);
    m_connected = QVector<bool>(m_wireCount, false);

    m_order.reserve(m_wireCount);
    for (int i = 0; i < m_wireCount; ++i) {
        m_order.append(i);
    }

    // Desna strana izmesana, ali nikad u istom redu kao leva.
    if (m_wireCount > 1) {
        do {
            std::shuffle(m_order.begin(), m_order.end(),
                         *QRandomGenerator::global());
        } while (std::is_sorted(m_order.cbegin(), m_order.cend()));
    }

    setFixedSize(460, 70 + m_wireCount * 65);
    setMouseTracking(true);
}

// ---------------------------------------------------------------- crtanje

void WireTaskWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    p.fillRect(rect(), QColor(40, 44, 52));
    p.setPen(QPen(QColor(90, 95, 105), 6));
    p.drawRect(rect().adjusted(3, 3, -3, -3));

    // Konektori levo i desno.
    for (int i = 0; i < m_wireCount; ++i) {
        drawPlug(p, leftEnd(i),  m_colors[i]);
        drawPlug(p, rightEnd(i), m_colors[m_order[i]]);
    }

    // Spojene zice.
    for (int i = 0; i < m_wireCount; ++i) {
        if (m_connected[i]) {
            drawWire(p, leftEnd(i), rightEnd(m_order.indexOf(i)), m_colors[i]);
        }
    }

    // Zica koju igrac upravo vuce.
    if (m_dragging >= 0) {
        drawWire(p, leftEnd(m_dragging), m_mouse, m_colors[m_dragging]);
    }

    p.setPen(Qt::white);
    QFont f = font();
    f.setPointSize(12);
    f.setBold(true);
    p.setFont(f);
    p.drawText(QRect(0, height() - 40, width(), 30), Qt::AlignCenter,
               m_solved ? QStringLiteral("Gotovo!")
                        : QStringLiteral("Spoji zice iste boje"));
}

// ---------------------------------------------------------------- mis

void WireTaskWidget::mousePressEvent(QMouseEvent* e)
{
    if (m_solved) {
        return;
    }
    for (int i = 0; i < m_wireCount; ++i) {
        if (!m_connected[i] && near(e->position(), leftEnd(i))) {
            m_dragging = i;
            m_mouse = e->position();
            update();
            return;
        }
    }
}

void WireTaskWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (m_dragging >= 0) {
        m_mouse = e->position();
        update();
    }
}

void WireTaskWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (m_dragging < 0) {
        return;
    }
    for (int slot = 0; slot < m_wireCount; ++slot) {
        // Spoj je dobar ako je na desnom mestu ista boja.
        if (near(e->position(), rightEnd(slot)) && m_order[slot] == m_dragging) {
            m_connected[m_dragging] = true;
            break;
        }
    }
    m_dragging = -1;
    update();

    if (!m_solved && allConnected()) {
        m_solved = true;
        update();
        emit solved();
    }
}

// ---------------------------------------------------------------- pomocno

QPointF WireTaskWidget::leftEnd(int i) const
{
    return { 50.0, 50.0 + i * 65.0 };
}

QPointF WireTaskWidget::rightEnd(int i) const
{
    return { width() - 50.0, 50.0 + i * 65.0 };
}

bool WireTaskWidget::allConnected() const
{
    return std::all_of(m_connected.cbegin(), m_connected.cend(),
                       [](bool c) { return c; });
}

bool WireTaskWidget::near(const QPointF& a, const QPointF& b)
{
    return QLineF(a, b).length() < 25.0;
}

void WireTaskWidget::drawPlug(QPainter& p, const QPointF& c, const QColor& color)
{
    p.setPen(QPen(Qt::black, 2));
    p.setBrush(color);
    p.drawRect(QRectF(c.x() - 14, c.y() - 10, 28, 20));
}

void WireTaskWidget::drawWire(QPainter& p, const QPointF& a, const QPointF& b,
                              const QColor& color)
{
    p.setPen(QPen(color, 10, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(a, b);
}
