#include "game/GameMap.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace
{
// Ove konstante su dostupne samo u ovom .cpp fajlu.
constexpr double PlacementRadius = 20.0; // Poluprečnik za proveru prostora na početnim pozicijama i stanicama.
constexpr int PlacementStep = 40;        // Razmak između tačaka koje proveravamo na maski.
constexpr int MaxTaskStations = 12;      // Najveći broj stanica za zadatke.
}

// Učitava slike i priprema početne pozicije i stanice za zadatke.
bool GameMap::load(const QString& backgroundPath, const QString& maskPath)
{
    // Učitavanje u privremeni objekat čuva prethodnu mapu ako učitavanje ne uspe.
    GameMap loaded;
    const QImage mask(maskPath);
    // Slika i maska moraju da postoje i imaju iste dimenzije.
    if (!loaded.m_background.load(backgroundPath) || mask.isNull() ||
        loaded.m_background.size() != mask.size()) {
        return false;
    }

    // Svaki piksel maske postaje jedan bajt svetline od 0 do 255.
    loaded.m_mask = mask.convertToFormat(QImage::Format_Grayscale8);
    if (loaded.m_mask.isNull()) {
        return false;
    }

    // Prohodne tačke sa mreže koristimo za privremeni raspored stanica.
    QVector<QPointF> candidates;
    for (int y = PlacementStep / 2; y < mask.height(); y += PlacementStep) {
        for (int x = PlacementStep / 2; x < mask.width(); x += PlacementStep) {
            const QPointF position(x, y);
            if (loaded.isWalkable(position, PlacementRadius)) {
                candidates.append(position);
            }
        }
    }

    // Biramo stanice u ravnomernim razmacima kroz listu prohodnih tačaka.
    const auto stationCount = std::min<qsizetype>(MaxTaskStations, candidates.size());
    for (qsizetype i = 0; i < stationCount; ++i) {
        loaded.m_stations.append(candidates.at(i * candidates.size() / stationCount));
    }

    // Pozicije oko centralnog stola, u pikselima originalne mape.
    if (mask.size() == QSize(2880, 1472)) {
        const QVector<QPointF> spawnPoints{
            {1540, 305}, // gore levo
            {1752, 305}, // gore desno
            {1767, 365}, // desno
            {1692, 458}, // dole desno
            {1600, 458}, // dole levo
            {1525, 365}, // levo
        };
        for (const QPointF& position : spawnPoints) {
            if (loaded.isWalkable(position, PlacementRadius)) {
                loaded.m_spawnPoints.append(position);
            }
        }
    }

    // Tek nakon uspešnog učitavanja preuzimamo sve podatke nove mape.
    *this = std::move(loaded);
    return true;
}

// Proverava da li krug igrača staje unutar mape bez presecanja prepreka.
bool GameMap::isWalkable(const QPointF& pos, double radius) const
{
    // Odbijamo neučitanu masku, nevažeće koordinate i negativan poluprečnik.
    if (m_mask.isNull() || !std::isfinite(pos.x()) || !std::isfinite(pos.y()) ||
        !std::isfinite(radius) || radius < 0.0) {
        return false;
    }

    // Granice pravougaonika koji obuhvata krug igrača.
    const double left = pos.x() - radius;
    const double right = pos.x() + radius;
    const double top = pos.y() - radius;
    const double bottom = pos.y() + radius;
    // Ceo krug mora da stane u mapu, a centar mora da bude unutar slike.
    if (left < 0.0 || top < 0.0 || right > m_mask.width() || bottom > m_mask.height() ||
        pos.x() >= m_mask.width() || pos.y() >= m_mask.height()) {
        return false;
    }

    // Vrednosti 0–127 označavaju prepreku, a 128–255 prohodan prostor.
    if (m_mask.constScanLine(static_cast<int>(pos.y()))[static_cast<int>(pos.x())] <= 127) {
        return false;
    }
    // Za poluprečnik nula dovoljna je provera piksela u centru.
    if (radius == 0.0) {
        return true;
    }

    // Pregledamo piksele u obuhvatnom pravougaoniku i proveravamo dodir sa zidovima.
    const double radiusSquared = radius * radius;
    for (int y = static_cast<int>(top); y < bottom; ++y) {
        const auto* row = m_mask.constScanLine(y); // Pristup pikselima jednog reda maske.
        const double dy = pos.y() - std::clamp(pos.y(), double(y), y + 1.0);
        for (int x = static_cast<int>(left); x < right; ++x) {
            if (row[x] > 127) {
                // Prohodan piksel ne može da blokira igrača.
                continue;
            }

            // Najbliža tačka na pikselu zida otkriva da li ga krug preseca.
            const double dx = pos.x() - std::clamp(pos.x(), double(x), x + 1.0);
            if (dx * dx + dy * dy < radiusSquared) {
                return false;
            }
        }
    }
    return true;
}

// Vraća pozadinu koju UI koristi za prikaz mape.
QPixmap GameMap::background() const
{
    return m_background;
}

// Vraća širinu i visinu učitane pozadine u pikselima.
QSize GameMap::size() const
{
    return m_background.size();
}

// Vraća početne pozicije sa kojih igrači mogu da krenu.
QVector<QPointF> GameMap::spawnPoints() const
{
    return m_spawnPoints;
}

// Vraća pozicije stanica na kojima se izvršavaju zadaci.
QVector<QPointF> GameMap::taskStations() const
{
    return m_stations;
}
