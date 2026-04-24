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

#endif // SMILEY_H
