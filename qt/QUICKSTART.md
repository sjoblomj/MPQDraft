# Quick Start Guide - MPQDraft Qt GUI

This guide will get you up and running with the Qt GUI development in minutes.

## Step 1: Install Dependencies

On Ubuntu 22.04 or later:

```bash
sudo apt-get update
sudo apt-get install -y \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    mingw-w64 \
    g++-mingw-w64-i686 \
    g++-mingw-w64-x86-64 \
    cmake \
    ninja-build \
    wine64
```

**Note:** If you're on Ubuntu 20.04 or older, Qt6 may not be available. You can either:
- Upgrade to Ubuntu 22.04+
- Install Qt6 from the [official Qt installer](https://www.qt.io/download-qt-installer)
- Use Qt5 instead (change `Qt6` to `Qt5` in CMakeLists.txt)

## Step 2: Build for Linux (Development)

This builds a native Linux version for quick testing:

```bash
cd qt
./build.sh linux
```

Run it:
```bash
./build/MPQDraftQt
```

**Note:** The Linux version is just for GUI development. The actual patching functionality requires Windows.

## Step 3: Set Up Qt6 for Cross-Compilation

**Important:** Cross-compiling to Windows requires Qt6 built for MinGW, not the Linux Qt6.

### Option A: Automatic Setup (Recommended)

Run the setup helper script:

```bash
cd qt
./setup-qt-mingw.sh
```

This will:
1. Search for Qt6 MinGW installations
2. Update the toolchain files automatically
3. Tell you which build command to use

### Option B: Manual Setup

1. **Download Qt with MinGW:**
   - Go to https://www.qt.io/download-qt-installer
   - Download the Qt Online Installer for Linux
   - Run it: `chmod +x qt-*.run && ./qt-*.run`
   - Select Qt 6.x.x
   - Check **"MinGW 11.2.0 32-bit"** and/or **"MinGW 11.2.0 64-bit"**
   - Install (default location: `~/Qt/`)

2. **Update toolchain files:**

   Edit `qt/toolchain/mingw-w64-i686.cmake` (for 32-bit):
   ```cmake
   set(Qt6_DIR "$ENV{HOME}/Qt/6.8.0/mingw_32/lib/cmake/Qt6")
   ```

   Edit `qt/toolchain/mingw-w64-x86_64.cmake` (for 64-bit):
   ```cmake
   set(Qt6_DIR "$ENV{HOME}/Qt/6.8.0/mingw_64/lib/cmake/Qt6")
   ```

   (Adjust version number to match your installation)

### Option C: Skip Cross-Compilation (For Now)

You can develop the GUI on Linux and cross-compile later:

```bash
./build.sh linux
./build/MPQDraftQt
```

The GUI will work on Linux (with Wine for testing), and you can add Windows cross-compilation when needed.

## Step 4: Cross-Compile for Windows

Once Qt6 MinGW is set up:

```bash
# For 32-bit Windows (recommended for compatibility)
./build.sh win32

# Or for 64-bit Windows
./build.sh win64
```

## Step 5: Test with Wine

Test the Windows build on Linux using Wine:

```bash
# For 32-bit
wine build-win32/MPQDraftQt.exe

# For 64-bit
wine64 build-win64/MPQDraftQt.exe
```

## Step 6: Test on Real Windows

Copy the executable to a Windows machine:

```bash
# The executable is self-contained
scp build-win32/MPQDraftQt.exe user@windows-machine:/path/to/destination/
```

## Troubleshooting

### Qt6 not found

**Error:** `Could not find a package configuration file provided by "Qt6"`

**Solution:** 
1. Check if Qt6 is installed: `dpkg -l | grep qt6`
2. If not, install it: `sudo apt-get install qt6-base-dev`
3. If using Qt installer, set Qt6_DIR: `export Qt6_DIR=~/Qt/6.x.x/gcc_64/lib/cmake/Qt6`

### MinGW not found

**Error:** `i686-w64-mingw32-g++: command not found`

**Solution:**
```bash
sudo apt-get install mingw-w64 g++-mingw-w64-i686 g++-mingw-w64-x86-64
```

### Wine crashes

**Error:** Wine crashes when running the executable

**Solution:**
1. Make sure Wine is properly installed: `wine --version`
2. Initialize Wine: `winecfg` (run once)
3. Try 64-bit Wine: `wine64` instead of `wine`

### Build errors

**Error:** Various build errors

**Solution:**
1. Clean and rebuild: `./build.sh clean && ./build.sh linux`
2. Check CMake version: `cmake --version` (need 3.16+)
3. Check compiler: `g++ --version` (need C++17 support)

## Development Workflow

### Typical Development Cycle

1. **Edit code** in your favorite editor (VS Code, Qt Creator, etc.)
2. **Build for Linux** for quick testing: `./build.sh linux`
3. **Test GUI** on Linux: `./build/MPQDraftQt`
4. **Cross-compile** when ready: `./build.sh win32`
5. **Test with Wine** or on real Windows

### Using Qt Creator (Optional)

Qt Creator provides a great IDE for Qt development:

```bash
sudo apt-get install qtcreator
qtcreator qt/CMakeLists.txt
```

In Qt Creator:
1. Open `qt/CMakeLists.txt`
2. Configure project with your Qt6 kit
3. Build and run with F5

## What's Next?

The current implementation provides:
- ✅ Main window with two buttons
- ✅ Patch wizard (3 pages)
- ✅ SEMPQ wizard (2 pages)
- ✅ Plugin loading and configuration

Still TODO:
- 🚧 Connect to actual patcher DLL
- 🚧 Implement SEMPQ creation
- 🚧 Add application icons
- 🚧 Load game registry entries
- 🚧 Progress dialogs
- 🚧 Error handling

See `README.md` for more details.

## Getting Help

If you run into issues:
1. Check the error message carefully
2. Look in `README.md` for more detailed information
3. Check Qt6 documentation: https://doc.qt.io/qt-6/
4. Check MinGW-w64 documentation: https://www.mingw-w64.org/

## Quick Reference

```bash
# Build commands
./build.sh linux    # Build for Linux
./build.sh win32    # Cross-compile for Windows 32-bit
./build.sh win64    # Cross-compile for Windows 64-bit
./build.sh clean    # Clean all builds

# Run commands
./build/MPQDraftQt                  # Run Linux build
wine build-win32/MPQDraftQt.exe     # Run Windows 32-bit with Wine
wine64 build-win64/MPQDraftQt.exe   # Run Windows 64-bit with Wine

# Development
qtcreator qt/CMakeLists.txt         # Open in Qt Creator
```

