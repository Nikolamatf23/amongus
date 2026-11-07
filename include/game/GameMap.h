#ifndef GAME_MAP_H
#define GAME_MAP_H

#include <QImage>
#include <QPixmap>
#include <QPointF>
#include <QSize>
#include <QString>
#include <QVector>

class GameMap
{
public:
    bool load(const QString& backgroundPath, const QString& maskPath); // Učitava pozadinu i masku. Vraća true ako uspe.
    bool isWalkable(const QPointF& pos, double radius) const;          // Proverava da li igrač datog poluprečnika staje na poziciju.

    QPixmap background() const;           // Vraća sliku mape za prikaz.
    QSize size() const;                    // Vraća širinu i visinu mape.
    QVector<QPointF> spawnPoints() const;  // Vraća početne pozicije igrača.
    QVector<QPointF> taskStations() const; // Vraća pozicije stanica za zadatke.

private:
    QPixmap m_background;           // Slika mape koju igrač vidi.
    QImage m_mask;                  // Maska prohodnosti. Belo je prohodno, crno je prepreka.
    QVector<QPointF> m_spawnPoints;  // Početne pozicije igrača.
    QVector<QPointF> m_stations;     // Pozicije stanica za zadatke.
};

#endif
