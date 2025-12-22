# MPQDraft
MPQDraft allows in-memory patching of game data files, added features through plugins, and creation of self-executing mods.

This fork aims to modernise MPQDraft for use on Win10 APIs and streamline the development process.

MPQDraft supports two modes:
1. Patching games locally: You run the game you choose with the MPQs and plugins that you specify. This mode is for playing mods.
2. SEMPQ (Self-Executing MPQ) creation: You create an executable with your mod that you can distribute to others. All they need to do is to run the executable; they do not need to have MPQDraft installed. This mode is for mod creators, wishing to make it easy for others to run their mods.


## Supported games
There is built in-support for the following games. Other 32-bit games that use MPQs and Storm.dll should theoretically also work.

- Diablo
- Diablo: Hellfire
- Diablo II
- Starcraft
- Starcraft Campaign Editor (StarEdit)
- Warcraft II: Battle.net Edition
- Warcraft III: Reign of Chaos
- Warcraft III: The Frozen Throne
- Warcraft III: World Editor
- Lords of Magic SE


## GUI
MPQDraft comes with a GUI written in Qt. To start it, simply run the MPQDraft executable without passing in any arguments. Passing in arguments will make it run in CLI mode.


## Command Line Usage

### Patch Command
To run a mod on the target you specify, use the `patch` command:

```
Usage: MPQDraft.exe patch --target <exePath> --mpq <mpqFile> --plugin <qdpFile>
```

Example:
```
MPQDraft.exe patch --target "C:\Starcraft\StarCraft.exe" --mpq "C:\Mod\my_mod_1.mpq" --mpq "C:\Mod\my_mod_2.mpq" --plugin "C:\Mod\my_plugin_1.qdp" --plugin "C:\Mod\my_plugin_2.qdp"
```

### SEMPQ Command

To create a SEMPQ (Self-Executing MPQ), use the `sempq` command. Three modes are available using the `sempq` command

- **Supported game mode**: Target one of the games that MPQDraft has built-in support for.
- **Custom registry mode**: The SEMPQ uses the Windows Registry to find the path of the game that will be launched on the end-users system. In case you want to target something that is not in the built list of supported games, you can use this mode to specify the target.
- **Custom target mode**: Here, a hard-coded path to an executable can be given. This requires the target application to be present on the end-users system under the exact given path.

#### Supported Game Mode

```
Usage: MPQDraft.exe sempq --output <sempqFile> --name <sempqName> --icon <iconFile> --mpq <mpqFile> --plugin <qdpFile> --game <game>
```

Example:
```
Usage: MPQDraft.exe sempq --output starwars_tc.exe --name "Star Wars TC" --icon starwars.ico --mpq sctc.mpq --plugin "C:\Mod\my_plugin_1.qdp" --game starcraft
```

For the `game` parameter, use `list-games` to see a list of supported games and their aliases (use any alias at your convenience to specify the game).

#### Custom Registry Mode

```
Usage: MPQDraft.exe sempq --output <sempqFile> --name <sempqName> --icon <iconFile> --mpq <mpqFile> --plugin <qdpFile> --reg-key <regKey> --reg-value <regValue> [--exe-file <exeFile>] [--target-file <targetFile>] [--full-path]
```

Example:
```
Usage: MPQDraft.exe sempq --output starwars_tc.exe --name "Star Wars TC" --icon starwars.ico --mpq sctc.mpq --plugin "C:\Mod\my_plugin_1.qdp" --reg-key "SOFTWARE\Blizzard Entertainment\SomeGame" --reg-value "InstallPath" --exe-file "SomeGame.exe" --target-file "SomeGame.exe"
```

#### Custom Target Mode

```
Usage: MPQDraft.exe sempq --output <sempqFile> --name <sempqName> --icon <iconFile> --mpq <mpqFile> --plugin <qdpFile> --target <targetPath>
```

Example:
```
Usage: MPQDraft.exe sempq --output starwars_tc.exe --name "Star Wars TC" --icon starwars.ico --mpq sctc.mpq --plugin "C:\Mod\my_plugin_1.qdp" --target "C:\SomeGame.exe"
```

### Patching Options
Additional options can also be passed on:

- `--params`: Command-line parameters to pass to the target executable. These are passed directly to the game and have no effect on MPQDraft itself.
- `--extended-redir` / `--no-extended-redir`: Enable or disable extended file redirection (default: enabled). Blizzard games use Storm.dll to access MPQ archives. Some Storm functions (like `SFileOpenFileEx`) can bypass the normal MPQ priority chain by accepting a specific archive handle. When enabled, MPQDraft hooks these functions to force them to search through the entire MPQ priority chain (including your custom MPQs), even when the game tries to read from a specific archive. Most Blizzard games including StarCraft and Warcraft III require this for mods to work correctly. Only disable if you experience compatibility issues.
- `--no-spawning`: Do not inject into child processes. By default, MPQDraft injects itself into any child processes created by the game, ensuring that patches work even if the game launches additional executables. Enable this flag if the game launches helper processes (updaters, launchers, crash reporters) that don't need patching and may cause issues.
- `--shunt-count`: The number of times the game restarts itself before MPQDraft activates patching (default: 0). Use 0 for most games to activate immediately. Some games with copy protection (like Diablo) restart themselves after checking the CD, so MPQDraft needs to wait for this restart - use 1 in such cases.


### CLI Plugin Configuration
MPQDraft plugins can optionally have configuration dialogs. The CLI contains no support for configuring plugins, but if one first runs MPQDraft in GUI mode, the plugins can be configured there, and those changes should persist when running in CLI mode.


## Development

### Building in Visual Studio
- Open src\CMakeLists.txt in Visual Studio
- Build->Build Solution

### Building with GitHub Actions
The .github/workflows/windows-build.yml file contains a build pipeline that will produce an executable. Clone the repo and push on any branch to have the build automatically run.

### Building under Linux

MPQDraft can be cross-compiled for 32-bit Windows from Linux using MinGW-w64.

#### Prerequisites

Install the MinGW-w64 toolchain:

```bash
sudo apt install mingw-w64 g++-mingw-w64-i686-posix
```

#### Building without Qt (DLL and Stub only)

To build just the DLL and Stub (no GUI):

```bash
./tools/build-mingw32.sh
```

Output files will be in `build-mingw32/`:
- `MPQDraftDLL.dll` - The injection DLL
- `MPQStub.exe` - The SEMPQ stub executable

#### Building with Qt GUI (using MXE)

To build the full application including the Qt GUI, you need Qt5 compiled for MinGW. The recommended approach is to use [MXE (M Cross Environment)](https://mxe.cc/).

1. **Install MXE dependencies** (Ubuntu/Debian):

```bash
sudo apt install autoconf automake autopoint bash bison bzip2 flex \
    g++ g++-multilib gettext git gperf intltool libc6-dev-i386 \
    libgdk-pixbuf2.0-dev libltdl-dev libgl-dev libpcre3-dev libssl-dev \
    libtool-bin libxml-parser-perl lzip make openssl p7zip-full patch \
    perl python3 python3-mako python3-packaging python-is-python3 ruby \
    sed unzip wget xz-utils
```

2. **Clone and build MXE with Qt5** (this takes 1-2 hours):

```bash
sudo git clone https://github.com/mxe/mxe.git /opt/mxe
cd /opt/mxe
sudo make MXE_TARGETS='i686-w64-mingw32.static' qt5
```

3. **Build MPQDraft with Qt**:

```bash
MXE_ROOT=/opt/mxe ./tools/build-mingw32.sh
```

The resulting executables will be statically linked and won't require Qt DLLs to be distributed alongside the application.

#### Alternative: Using a custom Qt installation

If you have Qt for MinGW installed elsewhere (e.g., from the Qt installer), you can specify its path:

```bash
QT_MINGW32_PREFIX=/path/to/Qt/5.15.2/mingw81_32 ./tools/build-mingw32.sh
```

Note: This may have compatibility issues if the Qt installation uses a different MinGW version than your system's.


## Credits and License
- [Justin Olbrantz (Quantam), creator of MPQDraft](http://qstuff.blogspot.com/2010/01/bibliography-programming.html).
- [milestone-dev, for adding CLI and doing a little modernising](https://github.com/milestone-dev/MPQDraft)
- [Johan Sjöblom (Ojan), for new GUI, major modernisation and refactoring, and cross-compilation support](https://github.com/sjoblomj/MPQDraft)

### Application
MPQDraft is licensed under CDDL.

### Third-party code

- The CLI uses [CLI11](https://github.com/CLIUtils/CLI11) (**src/app/cli/CLI11.hpp**) for command line parsing. CLI11 is licensed under BSD-3-Clause.

### Graphics and icons

- **src/app/gui/resources/images**: These were created by Joel Steudler for use in MPQDraft. Not for commercial use.
- **src/app/gui/resources/icons/blizzard**: These are from the games they represent, and owned by Blizzard Entertainment. Not for commercial use.
- **src/app/gui/resources/icons/sierra**: These are from the games they represent, and owned by Sierra Entertainment. Not for commercial use.
- **src/app/gui/resources/icons/StarDraft.{png,ico}**: This was taken from StarDraft, a predecessor of MPQDraft, written by separate authors, where it was used as the icon for Self-Executing CWADs (a precursor to Self-Executing MPQs). To be honest, it is unclear where this icon originated from, or who created it. Best not used commercially.
- **src/app/gui/resources/icons/***: Public domain icons.
