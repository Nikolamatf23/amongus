#include "game/tasks/WireTask.h"

#include <QLineF>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QRandomGenerator>
#include <QWidget>

#include <algorithm>

// ============================================================================
//  WireTaskWidget - pomocni widget koji vidi samo ovaj .cpp fajl.
//  Dobija boje i raspored od WireTask-a, crta panel i prati mis.
//  Kad su sve zice spojene, zove task->markCompleted().
// ============================================================================
namespace
{

constexpr int WIRE_COUNT = 4;

class WireTaskWidget : public QWidget
{
public:
    WireTaskWidget(Task* task,
                   const QVector<QColor>& colors,
                   const QVector<int>& order)
        : m_task(task)
        , m_colors(colors)
        , m_order(order)
        , m_connected(WIRE_COUNT, false)
    {
        setFixedSize(460, 340);
        setMouseTracking(true);
        setWindowTitle(task->name());
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        p.fillRect(rect(), QColor(40, 44, 52));
        p.setPen(QPen(QColor(90, 95, 105), 6));
        p.drawRect(rect().adjusted(3, 3, -3, -3));

        // Konektori levo i desno.
        for (int i = 0; i < WIRE_COUNT; ++i) {
            drawPlug(p, leftEnd(i),  m_colors[i]);
            drawPlug(p, rightEnd(i), m_colors[m_order[i]]);
        }

        // Spojene zice.
        for (int i = 0; i < WIRE_COUNT; ++i) {
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
        const bool done = !m_task.isNull() && m_task->isCompleted();
        p.drawText(QRect(0, height() - 40, width(), 30), Qt::AlignCenter,
                   done ? QStringLiteral("Gotovo!")
                        : QStringLiteral("Spoji zice iste boje"));
    }

    void mousePressEvent(QMouseEvent* e) override
    {
        if (m_task.isNull() || m_task->isCompleted()) {
            return;
        }
        for (int i = 0; i < WIRE_COUNT; ++i) {
            if (!m_connected[i] && near(e->position(), leftEnd(i))) {
                m_dragging = i;
                m_mouse = e->position();
                update();
                return;
            }
        }
    }

    void mouseMoveEvent(QMouseEvent* e) override
    {
        if (m_dragging >= 0) {
            m_mouse = e->position();
            update();
        }
    }

    void mouseReleaseEvent(QMouseEvent* e) override
    {
        if (m_dragging < 0) {
            return;
        }
        for (int slot = 0; slot < WIRE_COUNT; ++slot) {
            // Spoj je dobar ako je na desnom mestu ista boja.
            if (near(e->position(), rightEnd(slot)) &&
                m_order[slot] == m_dragging) {
                m_connected[m_dragging] = true;
                break;
            }
        }
        m_dragging = -1;
        update();

        const bool all = std::all_of(m_connected.cbegin(), m_connected.cend(),
                                     [](bool c) { return c; });
        if (all && !m_task.isNull()) {
            m_task->markCompleted();      // -> signal Task::completed(id)
        }
    }

private:
    QPointF leftEnd(int i) const  { return { 50.0, 50.0 + i * 65.0 }; }
    QPointF rightEnd(int i) const { return { width() - 50.0, 50.0 + i * 65.0 }; }

    static bool near(const QPointF& a, const QPointF& b)
    {
        return QLineF(a, b).length() < 25.0;
    }

    static void drawPlug(QPainter& p, const QPointF& c, const QColor& color)
    {
        p.setPen(QPen(Qt::black, 2));
        p.setBrush(color);
        p.drawRect(QRectF(c.x() - 14, c.y() - 10, 28, 20));
    }

    static void drawWire(QPainter& p, const QPointF& a, const QPointF& b,
                         const QColor& color)
    {
        p.setPen(QPen(color, 10, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(a, b);
    }

    QPointer<Task>  m_task;        // postaje nullptr ako se task obrise
    QVector<QColor> m_colors;
    QVector<int>    m_order;
    QVector<bool>   m_connected;
    int             m_dragging = -1;
    QPointF         m_mouse;
};

} // namespace

// ============================================================================
//  WireTask
// ============================================================================

WireTask::WireTask(int id,
                   const QString& name,
                   const QPointF& stationPos,
                   QObject* parent)
    : Task(id, name, stationPos, parent)
    , m_wireColors({ QColor(220, 40, 40),     // crvena
                     QColor(40, 90, 230),     // plava
                     QColor(240, 220, 40),    // zuta
                     QColor(230, 60, 200) })  // roze
    , m_order({ 0, 1, 2, 3 })
{
}

QWidget* WireTask::createWidget()
{
    // Svako otvaranje - novi raspored desne strane (nikad 0,1,2,3).
    do {
        std::shuffle(m_order.begin(), m_order.end(),
                     *QRandomGenerator::global());
    } while (std::is_sorted(m_order.cbegin(), m_order.cend()));

    return new WireTaskWidget(this, m_wireColors, m_order);
}
