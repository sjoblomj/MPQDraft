#ifndef GAMEDATA_QT_H
#define GAMEDATA_QT_H

#include "core/gamedata.h"
#include "core/gamedetection.h"
#include <QString>
#include <QVector>

// Qt wrapper functions for game data and detection.
// These provide Qt-friendly interfaces to the standard C++ game data functions

// Convert GameComponent to Qt-friendly version (in-place conversion helpers)
inline QString getComponentName(const GameComponent& comp) {
    return QString::fromStdString(comp.componentName);
}

inline QString getFileName(const GameComponent& comp) {
    return QString::fromStdString(comp.fileName);
}

inline QString getTargetFileName(const GameComponent& comp) {
    return QString::fromStdString(comp.targetFileName);
}

inline QString getIconPath(const GameComponent& comp) {
    return QString::fromStdString(comp.iconPath);
}

// Convert SupportedGame to Qt-friendly version (in-place conversion helpers)
inline QString getGameName(const SupportedGame& game) {
    return QString::fromStdString(game.gameName);
}

inline QString getRegistryKey(const SupportedGame& game) {
    return QString::fromStdString(game.registryKey);
}

inline QString getRegistryValue(const SupportedGame& game) {
    return QString::fromStdString(game.registryValue);
}

inline QVector<SupportedGame> getSupportedGamesQt() {
    std::vector<SupportedGame> games = getSupportedGames();
    QVector<SupportedGame> qGames;
    for (const SupportedGame& game : games) {
        qGames.append(game);
    }
    return qGames;
}

#endif // GAMEDATA_QT_H
