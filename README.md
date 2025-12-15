# MPQDraft
MPQDraft allows in-memory patching of game data files, added features through plugins, and creation of self-executing mods.

This fork aims to modernise MPQDraft for use on Win10 APIs and streamline the development process.

## Command Line Usage
```
Usage: MPQDraft.exe --target <exePath> --mpq <mpqFile> --plugin <qdpFile>

Example:
MPQDraft.exe --target "C:\Starcraft\StarCraft.exe" --mpq "C:\Mod\my_mod_1.mpq" --mpq "C:\Mod\my_mod_2.mpq" --plugin "C:\Mod\my_plugin_1.qdp" --plugin "C:\Mod\my_plugin_2.qdp"
```

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


## Credits
- [Justin Olbrantz (Quantam), creator of MPQDraft](http://qstuff.blogspot.com/2010/01/bibliography-programming.html).
- [milestone-dev, for adding CLI and doing a little modernising](https://github.com/milestone-dev/MPQDraft)
- [Johan Sjöblom (Ojan), for new GUI, major modernisation and refactoring, and cross-compilation support](https://github.com/sjoblomj/MPQDraft)

### Application
MPQDraft is licensed under CDDL.

### Graphics and icons

- **src/app/gui/resources/images**: These were created by Joel Steudler for use in MPQDraft. Not for commercial use.
- **src/app/gui/resources/icons/blizzard**: These are from the games they represent, and owned by Blizzard Entertainment. Not for commercial use.
- **src/app/gui/resources/icons/sierra**: These are from the games they represent, and owned by Sierra Entertainment. Not for commercial use.
- **src/app/gui/resources/icons/StarDraft.{png,ico}**: This was taken from StarDraft, a predecessor of MPQDraft, written by separate authors, where it was used as the icon for Self-Executing CWADs (a precursor to Self-Executing MPQs). To be honest, it is unclear where this icon originated from, or who created it. Best not used commercially.
- **src/app/gui/resources/icons/***: Public domain icons
