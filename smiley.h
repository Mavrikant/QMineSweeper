#ifndef SMILEY_H
#define SMILEY_H

#include "minefield.h"

#include <QString>

// Maps a MineField GameState to the emoji displayed on the header's status
// button. Kept header-only and free-standing so the unit test can exercise it
// without pulling mainwindow.cpp (and its ui/telemetry/language dependencies)
// into the test target.
inline QString smileyForState(GameState state)
{
    switch (state)
    {
    case GameState::Won:
        return QStringLiteral("😎");
    case GameState::Lost:
        return QStringLiteral("😵");
    case GameState::Ready:
    case GameState::Playing:
        return QStringLiteral("🙂");
    }
    return QStringLiteral("🙂");
}

// Tension-aware variant: while a cell is held down during Ready/Playing, the
// classic Minesweeper clones show a 😮 "holding my breath" face. Won/Lost
// states override tension — once the game is over, the cells are frozen and
// the indicator should stay on its final face.
inline QString smileyForTensionState(GameState state, bool pressing)
{
    if (pressing && (state == GameState::Ready || state == GameState::Playing))
    {
        return QStringLiteral("😮");
    }
    return smileyForState(state);
}

#endif // SMILEY_H
