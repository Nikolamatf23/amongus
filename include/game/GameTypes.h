#ifndef GAMETYPES_H
#define GAMETYPES_H

// ============================================================================
//  GameTypes.h
//  Enumeracije iz dijagrama "Model (logika igre)": Role, GamePhase, Winner.
//  Ukljucuju ga sve klase modela, ali i oba kontrolera.
// ============================================================================

#include <QMetaType>

// Uloga igraca.
enum class Role
{
    Crewmate,
    Impostor
};

// Faza partije. MainWindow na promenu faze prebacuje ekran.
enum class GamePhase
{
    Lobby,
    Playing,
    Discussion,
    Voting,
    GameOver
};

// Pobednik. Vraca ga GameState::checkWinCondition().
enum class Winner
{
    None,
    Crewmates,
    Impostors
};

// Da bi enumi mogli da idu kao parametri signala
// (npr. ClientController::phaseChanged(GamePhase), gameOver(Winner)).
Q_DECLARE_METATYPE(Role)
Q_DECLARE_METATYPE(GamePhase)
Q_DECLARE_METATYPE(Winner)

#endif // GAMETYPES_H
