#include "game/GameMap.h"

#include <QLineF>
#include <QRandomGenerator>
#include <QtMath>

#include <limits>

namespace
{
constexpr int     SPAWN_COUNT   = 10;     // max broj igraca
constexpr int     STATION_COUNT = 12;
constexpr double  SPAWN_RADIUS  = 80.0;   // krug spawn tacaka oko centra
constexpr double  CHECK_RADIUS  = 20.0;   // tacka mora da ima mesta za igraca
constexpr quint32 MAP_SEED      = 2026;   // isti seed -> iste stanice svaki put
}

bool GameMap::load(const QString& jpgPath, const QString& maskPath)
{
    QPixmap background;
    QImage  mask;
    if (!background.load(jpgPath) || !mask.load(maskPath)) {
        return false;
    }

    m_background = background;
    // Maska u sivim tonovima (1 bajt po pikselu), iste velicine kao slika.
    m_mask = mask.convertToFormat(QImage::Format_Grayscale8)
                 .scaled(m_background.size());

    // ---- sve prohodne tacke na mrezi od 40 px ---------------------------
    QVector<QPointF> candidates;
    for (int y = 20; y < m_mask.height(); y += 40) {
        for (int x = 20; x < m_mask.width(); x += 40) {
            if (isWalkable(QPointF(x, y), CHECK_RADIUS)) {
                candidates.append(QPointF(x, y));
            }
        }
    }
    m_spawnPoints.clear();
    m_stations.clear();
    if (candidates.isEmpty()) {
        return true;
    }

    // ---- spawn: krug oko centra, ili najbliza prohodna tacka -----------
    const QPointF center(m_mask.width() / 2.0, m_mask.height() / 2.0);
    for (int i = 0; i < SPAWN_COUNT; ++i) {
        const double a = 2.0 * M_PI * i / SPAWN_COUNT;
        QPointF p = center + QPointF(SPAWN_RADIUS * qCos(a), SPAWN_RADIUS * qSin(a));
        if (!isWalkable(p, CHECK_RADIUS)) {
            QPointF best = candidates.first();
            for (const QPointF& c : candidates) {
                if (QLineF(p, c).length() < QLineF(p, best).length()) {
                    best = c;
                }
            }
            p = best;
        }
        m_spawnPoints.append(p);
    }

    // ---- stanice: svaka nova je sto dalje od vec izabranih --------------
    QRandomGenerator rng(MAP_SEED);
    m_stations.append(candidates[rng.bounded(candidates.size())]);
    while (m_stations.size() < STATION_COUNT &&
           m_stations.size() < candidates.size()) {
        QPointF best;
        double bestDist = -1.0;
        for (const QPointF& c : candidates) {
            double nearest = std::numeric_limits<double>::max();
            for (const QPointF& s : m_stations) {
                nearest = qMin(nearest, QLineF(c, s).length());
            }
            if (nearest > bestDist) {
                bestDist = nearest;
                best = c;
            }
        }
        m_stations.append(best);
    }
    return true;
}

bool GameMap::isWalkable(const QPointF& pos, double radius) const
{
    if (m_mask.isNull()) {
        return true;              // mapa jos nije ucitana - nema zidova
    }

    // Proveravamo centar i 8 tacaka na obodu kruga.
    for (int i = -1; i < 8; ++i) {
        QPointF p = pos;
        if (i >= 0) {
            const double a = i * M_PI / 4.0;
            p += QPointF(radius * qCos(a), radius * qSin(a));
        }
        const int x = qRound(p.x());
        const int y = qRound(p.y());
        if (x < 0 || y < 0 || x >= m_mask.width() || y >= m_mask.height()) {
            return false;         // van mape
        }
        if (m_mask.constScanLine(y)[x] <= 128) {
            return false;         // crno = zid
        }
    }
    return true;
}

QPixmap GameMap::background() const
{
    return m_background;
}

QSize GameMap::size() const
{
    return m_background.size();
}

QVector<QPointF> GameMap::spawnPoints() const
{
    return m_spawnPoints;
}

QVector<QPointF> GameMap::taskStations() const
{
    return m_stations;
}
