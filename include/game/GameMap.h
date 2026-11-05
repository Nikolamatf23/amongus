#ifndef GAMEMAP_H
#define GAMEMAP_H

// ============================================================================
//  GameMap
//  Mapa: jpg za prikaz + crno-bela maska za kolizije
//  (BELO = moze da se hoda, CRNO = zid / van mape).
//
//  Dijagram:
//    - m_background: QPixmap          - m_mask: QImage
//    - m_spawnPoints: QVector<QPointF> - m_stations: QVector<QPointF>
//    + load(jpgPath, maskPath): bool
//    + isWalkable(pos, radius): bool
//    + background(): QPixmap
//    + size(): QSize
//    + spawnPoints()
//    + taskStations()
//
//  Spawn tacke i stanice za taskove load() sam pronalazi na maski.
// ============================================================================

#include <QImage>
#include <QPixmap>
#include <QPointF>
#include <QSize>
#include <QString>
#include <QVector>

class GameMap
{
public:
    GameMap() = default;

    bool load(const QString& jpgPath, const QString& maskPath);

    // Da li krug poluprecnika 'radius' oko 'pos' staje na belo na maski.
    bool isWalkable(const QPointF& pos, double radius) const;

    QPixmap background() const;
    QSize   size() const;

    QVector<QPointF> spawnPoints() const;
    QVector<QPointF> taskStations() const;

private:
    QPixmap          m_background;
    QImage           m_mask;
    QVector<QPointF> m_spawnPoints;
    QVector<QPointF> m_stations;
};

#endif // GAMEMAP_H
