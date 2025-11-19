#include "gamedata.h"

// Static data for all supported games
static QVector<SupportedGame> supportedGames;
static bool initialized = false;

void initializeSupportedGames() {
    if (initialized) return;
    initialized = true;

    // Diablo
    {
        SupportedGame game;
        game.gameName = "Diablo";
        game.registryKey = "SOFTWARE\\Blizzard Entertainment\\Archives";
        game.registryValue = "DiabloInstall";
        game.iconPath = ":/icons/Diablo.ico";
        
        GameComponent comp;
        comp.componentName = "Diablo";
        comp.fileName = "Diablo.exe";
        comp.targetFileName = "Diablo.exe";
        comp.shuntCount = 1;
        comp.iconPath = ":/icons/Diablo.ico";
        comp.extendedRedir = true;
        comp.flags = 0;
        game.components.append(comp);
        
        supportedGames.append(game);
    }

    // Diablo: Hellfire
    {
        SupportedGame game;
        game.gameName = "Diablo: Hellfire";
        game.registryKey = "SOFTWARE\\Sierra OnLine\\Setup\\HELLFIRE";
        game.registryValue = "Directory";
        game.iconPath = ":/icons/hellfire.ico";
        
        GameComponent comp;
        comp.componentName = "Diablo: Hellfire";
        comp.fileName = "Hellfire.exe";
        comp.targetFileName = "Hellfire.exe";
        comp.shuntCount = 1;
        comp.iconPath = ":/icons/hellfire.ico";
        comp.extendedRedir = true;
        comp.flags = 0;
        game.components.append(comp);
        
        supportedGames.append(game);
    }

    // Diablo II
    {
        SupportedGame game;
        game.gameName = "Diablo II";
        game.registryKey = "SOFTWARE\\Blizzard Entertainment\\Diablo II";
        game.registryValue = "InstallPath";
        game.iconPath = ":/icons/Diablo2.ico";
        
        GameComponent comp;
        comp.componentName = "Diablo II";
        comp.fileName = "Diablo II.exe";
        comp.targetFileName = "Game.exe";
        comp.shuntCount = 0;
        comp.iconPath = ":/icons/Diablo2.ico";
        comp.extendedRedir = false;
        comp.flags = 0;
        game.components.append(comp);
        
        supportedGames.append(game);
    }

    // Starcraft
    {
        SupportedGame game;
        game.gameName = "Starcraft";
        game.registryKey = "SOFTWARE\\Blizzard Entertainment\\Starcraft";
        game.registryValue = "InstallPath";
        game.iconPath = ":/icons/Starcraft.ico";
        
        GameComponent comp1;
        comp1.componentName = "Starcraft";
        comp1.fileName = "Starcraft.exe";
        comp1.targetFileName = "Starcraft.exe";
        comp1.shuntCount = 0;
        comp1.iconPath = ":/icons/Starcraft.ico";
        comp1.extendedRedir = true;
        comp1.flags = 0;
        game.components.append(comp1);
        
        GameComponent comp2;
        comp2.componentName = "Campaign Editor";
        comp2.fileName = "StarEdit.exe";
        comp2.targetFileName = "StarEdit.exe";
        comp2.shuntCount = 0;
        comp2.iconPath = ":/icons/StarEdit.ico";
        comp2.extendedRedir = true;
        comp2.flags = 0;
        game.components.append(comp2);
        
        supportedGames.append(game);
    }

    // Warcraft II BNE
    {
        SupportedGame game;
        game.gameName = "Warcraft II BNE";
        game.registryKey = "SOFTWARE\\Blizzard Entertainment\\Warcraft II BNE";
        game.registryValue = "InstallPath";
        game.iconPath = ":/icons/War2BNE.ico";
        
        GameComponent comp;
        comp.componentName = "Warcraft II BNE";
        comp.fileName = "Warcraft II BNE.exe";
        comp.targetFileName = "Warcraft II BNE.exe";
        comp.shuntCount = 0;
        comp.iconPath = ":/icons/War2BNE.ico";
        comp.extendedRedir = true;
        comp.flags = 0;
        game.components.append(comp);
        
        supportedGames.append(game);
    }

    // Warcraft III
    {
        SupportedGame game;
        game.gameName = "Warcraft III";
        game.registryKey = "SOFTWARE\\Blizzard Entertainment\\Warcraft III";
        game.registryValue = "InstallPath";
        game.iconPath = ":/icons/War3Head.ico";
        
        GameComponent comp1;
        comp1.componentName = "Reign of Chaos";
        comp1.fileName = "Warcraft III.exe";
        comp1.targetFileName = "War3.exe";
        comp1.shuntCount = 0;
        comp1.iconPath = ":/icons/War3.ico";
        comp1.extendedRedir = true;
        comp1.flags = 0;
        game.components.append(comp1);
        
        GameComponent comp2;
        comp2.componentName = "The Frozen Throne";
        comp2.fileName = "Frozen Throne.exe";
        comp2.targetFileName = "War3.exe";
        comp2.shuntCount = 0;
        comp2.iconPath = ":/icons/War3x.ico";
        comp2.extendedRedir = true;
        comp2.flags = 0;
        game.components.append(comp2);
        
        GameComponent comp3;
        comp3.componentName = "World Editor";
        comp3.fileName = "World Editor.exe";
        comp3.targetFileName = "WorldEdit.exe";
        comp3.shuntCount = 0;
        comp3.iconPath = ":/icons/War3Edit.ico";
        comp3.extendedRedir = true;
        comp3.flags = 0;
        game.components.append(comp3);
        
        supportedGames.append(game);
    }

    // Lords of Magic SE
    {
        SupportedGame game;
        game.gameName = "Lords of Magic SE";
        game.registryKey = "SOFTWARE\\Sierra OnLine\\Setup\\LOMSE";
        game.registryValue = "Directory";
        game.iconPath = ":/icons/lomse.ico";
        
        GameComponent comp;
        comp.componentName = "Lords of Magic SE";
        comp.fileName = "LOMSE.exe";
        comp.targetFileName = "LOMSE.exe";
        comp.shuntCount = 1;
        comp.iconPath = ":/icons/lomse.ico";
        comp.extendedRedir = true;
        comp.flags = 0;
        game.components.append(comp);
        
        supportedGames.append(game);
    }
}

const QVector<SupportedGame>& getSupportedGames() {
    initializeSupportedGames();
    return supportedGames;
}

