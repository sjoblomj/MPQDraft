#include "GameData.h"

std::vector<SupportedGame> getSupportedGames() {
    std::vector<SupportedGame> games;

    // Diablo
    {
        SupportedGame game;
        game.gameName = "Diablo";
        game.registryKey = "SOFTWARE\\Blizzard Entertainment\\Archives";
        game.registryValue = "DiabloInstall";
        game.iconPath = ":/icons/blizzard/Diablo.ico";

        GameComponent comp;
        comp.componentName = "Diablo";
        comp.fileName = "Diablo.exe";
        comp.targetFileName = "Diablo.exe";
        comp.shuntCount = 1;
        comp.iconPath = ":/icons/blizzard/Diablo.ico";
        comp.extendedRedir = true;
        comp.flags = 0;
        game.components.push_back(comp);

        games.push_back(game);
    }

    // Diablo: Hellfire
    {
        SupportedGame game;
        game.gameName = "Diablo: Hellfire";
        game.registryKey = "SOFTWARE\\Sierra OnLine\\Setup\\HELLFIRE";
        game.registryValue = "Directory";
        game.iconPath = ":/icons/sierra/hellfire.ico";

        GameComponent comp;
        comp.componentName = "Diablo: Hellfire";
        comp.fileName = "Hellfire.exe";
        comp.targetFileName = "Hellfire.exe";
        comp.shuntCount = 1;
        comp.iconPath = ":/icons/sierra/hellfire.ico";
        comp.extendedRedir = true;
        comp.flags = 0;
        game.components.push_back(comp);

        games.push_back(game);
    }

    // Diablo II
    {
        SupportedGame game;
        game.gameName = "Diablo II";
        game.registryKey = "SOFTWARE\\Blizzard Entertainment\\Diablo II";
        game.registryValue = "InstallPath";
        game.iconPath = ":/icons/blizzard/Diablo2.ico";

        GameComponent comp;
        comp.componentName = "Diablo II";
        comp.fileName = "Diablo II.exe";
        comp.targetFileName = "Game.exe";
        comp.shuntCount = 0;
        comp.iconPath = ":/icons/blizzard/Diablo2.ico";
        comp.extendedRedir = false;
        comp.flags = 0;
        game.components.push_back(comp);

        games.push_back(game);
    }

    // Starcraft
    {
        SupportedGame game;
        game.gameName = "Starcraft";
        game.registryKey = "SOFTWARE\\Blizzard Entertainment\\Starcraft";
        game.registryValue = "InstallPath";
        game.iconPath = ":/icons/blizzard/Starcraft.ico";

        GameComponent comp1;
        comp1.componentName = "Starcraft";
        comp1.fileName = "Starcraft.exe";
        comp1.targetFileName = "Starcraft.exe";
        comp1.shuntCount = 0;
        comp1.iconPath = ":/icons/blizzard/Starcraft.ico";
        comp1.extendedRedir = true;
        comp1.flags = 0;
        game.components.push_back(comp1);

        GameComponent comp2;
        comp2.componentName = "Campaign Editor";
        comp2.fileName = "StarEdit.exe";
        comp2.targetFileName = "StarEdit.exe";
        comp2.shuntCount = 0;
        comp2.iconPath = ":/icons/blizzard/StarEdit.ico";
        comp2.extendedRedir = true;
        comp2.flags = 0;
        game.components.push_back(comp2);

        games.push_back(game);
    }

    // Warcraft II BNE
    {
        SupportedGame game;
        game.gameName = "Warcraft II BNE";
        game.registryKey = "SOFTWARE\\Blizzard Entertainment\\Warcraft II BNE";
        game.registryValue = "InstallPath";
        game.iconPath = ":/icons/blizzard/War2BNE.ico";

        GameComponent comp;
        comp.componentName = "Warcraft II BNE";
        comp.fileName = "Warcraft II BNE.exe";
        comp.targetFileName = "Warcraft II BNE.exe";
        comp.shuntCount = 0;
        comp.iconPath = ":/icons/blizzard/War2BNE.ico";
        comp.extendedRedir = true;
        comp.flags = 0;
        game.components.push_back(comp);

        games.push_back(game);
    }

    // Warcraft III
    {
        SupportedGame game;
        game.gameName = "Warcraft III";
        game.registryKey = "SOFTWARE\\Blizzard Entertainment\\Warcraft III";
        game.registryValue = "InstallPath";
        game.iconPath = ":/icons/blizzard/War3Head.ico";

        GameComponent comp1;
        comp1.componentName = "Reign of Chaos";
        comp1.fileName = "Warcraft III.exe";
        comp1.targetFileName = "War3.exe";
        comp1.shuntCount = 0;
        comp1.iconPath = ":/icons/blizzard/War3.ico";
        comp1.extendedRedir = true;
        comp1.flags = 0;
        game.components.push_back(comp1);

        GameComponent comp2;
        comp2.componentName = "The Frozen Throne";
        comp2.fileName = "Frozen Throne.exe";
        comp2.targetFileName = "War3.exe";
        comp2.shuntCount = 0;
        comp2.iconPath = ":/icons/blizzard/War3x.ico";
        comp2.extendedRedir = true;
        comp2.flags = 0;
        game.components.push_back(comp2);

        GameComponent comp3;
        comp3.componentName = "World Editor";
        comp3.fileName = "World Editor.exe";
        comp3.targetFileName = "WorldEdit.exe";
        comp3.shuntCount = 0;
        comp3.iconPath = ":/icons/blizzard/War3Edit.ico";
        comp3.extendedRedir = true;
        comp3.flags = 0;
        game.components.push_back(comp3);

        games.push_back(game);
    }

    // Lords of Magic SE
    {
        SupportedGame game;
        game.gameName = "Lords of Magic SE";
        game.registryKey = "SOFTWARE\\Sierra OnLine\\Setup\\LOMSE";
        game.registryValue = "Directory";
        game.iconPath = ":/icons/sierra/lomse.ico";

        GameComponent comp;
        comp.componentName = "Lords of Magic SE";
        comp.fileName = "LOMSE.exe";
        comp.targetFileName = "LOMSE.exe";
        comp.shuntCount = 1;
        comp.iconPath = ":/icons/sierra/lomse.ico";
        comp.extendedRedir = true;
        comp.flags = 0;
        game.components.push_back(comp);

        games.push_back(game);
    }

    return games;
}
