#ifndef GAMEDATA_QT_H
#define GAMEDATA_QT_H

#include "common/gamedata.h"
#include "common/gamedetection.h"
#include <QString>
#include <QVector>

// Qt wrapper functions for game data and detection
// These provide Qt-friendly interfaces to the standard C++ game data functions

// Convert std::string to QString
inline QString toQString(const std::string& str) {
    return QString::fromStdString(str);
}

// Convert QString to std::string
inline std::string toStdString(const QString& qstr) {
    return qstr.toStdString();
}

// Convert GameComponent to Qt-friendly version (in-place conversion helpers)
inline QString getComponentName(const GameComponent& comp) {
    return toQString(comp.componentName);
}

inline QString getFileName(const GameComponent& comp) {
    return toQString(comp.fileName);
}

inline QString getTargetFileName(const GameComponent& comp) {
    return toQString(comp.targetFileName);
}

inline QString getIconPath(const GameComponent& comp) {
    return toQString(comp.iconPath);
}

// Convert SupportedGame to Qt-friendly version (in-place conversion helpers)
inline QString getGameName(const SupportedGame& game) {
    return toQString(game.gameName);
}

inline QString getRegistryKey(const SupportedGame& game) {
    return toQString(game.registryKey);
}

inline QString getRegistryValue(const SupportedGame& game) {
    return toQString(game.registryValue);
}

inline QString getGameIconPath(const SupportedGame& game) {
    return toQString(game.iconPath);
}

// Qt-friendly wrappers for detection functions
inline bool locateGameQt(const QString& registryKey, const QString& registryValue) {
    return locateGame(toStdString(registryKey), toStdString(registryValue));
}

inline QString locateComponentQt(const QString& registryKey, const QString& registryValue, const QString& fileName) {
    return toQString(locateComponent(toStdString(registryKey), toStdString(registryValue), toStdString(fileName)));
}

inline QVector<const SupportedGame*> getInstalledGamesQt() {
    std::vector<const SupportedGame*> games = getInstalledGames();
    QVector<const SupportedGame*> qGames;
    for (const SupportedGame* game : games) {
        qGames.append(game);
    }
    return qGames;
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
