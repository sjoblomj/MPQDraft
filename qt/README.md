# MPQDraft Qt GUI

This is the Qt6-based GUI for MPQDraft, replacing the old MFC GUI.

## Building on Linux

### Prerequisites

Install Qt6 and MinGW-w64:

```bash
# Ubuntu 22.04+
sudo apt-get update
sudo apt-get install -y \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    libqt6core6 \
    libqt6gui6 \
    libqt6widgets6 \
    mingw-w64 \
    g++-mingw-w64-i686 \
    g++-mingw-w64-x86-64 \
    cmake \
    ninja-build \
    wine64
```

### Building for Linux (Development/Testing)

```bash
cd qt
mkdir build
cd build
cmake ..
cmake --build .
./MPQDraftQt
```

### Cross-Compiling for Windows (32-bit)

```bash
cd qt
mkdir build-win32
cd build-win32
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain/mingw-w64-i686.cmake ..
cmake --build .

# Test with Wine
wine MPQDraftQt.exe
```

### Cross-Compiling for Windows (64-bit)

```bash
cd qt
mkdir build-win64
cd build-win64
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain/mingw-w64-x86_64.cmake ..
cmake --build .

# Test with Wine
wine64 MPQDraftQt.exe
```

## Project Structure

```
qt/
├── CMakeLists.txt              # Main build configuration
├── README.md                   # This file
├── src/
│   ├── main.cpp                # Application entry point
│   ├── mainwindow.cpp/h        # Main menu window
│   ├── patchwizard.cpp/h       # Patch wizard (3 pages)
│   ├── sempqwizard.cpp/h       # SEMPQ wizard (3 pages)
│   ├── pluginpage.cpp/h        # Plugin selection page
│   └── common/                 # Shared code
│       ├── pluginloader.cpp/h  # Plugin loading utilities
│       ├── mpqdraftplugin.h    # Plugin interface definitions
│       └── common.h            # Common types and definitions
├── resources/
│   ├── mpqdraft.qrc            # Qt resource file
│   ├── images/
│   │   └── wizard.png          # Wizard sidebar image
│   └── icons/                  # Application icons (to be added)
└── toolchain/
    ├── mingw-w64-i686.cmake    # 32-bit Windows cross-compilation
    └── mingw-w64-x86_64.cmake  # 64-bit Windows cross-compilation
```

## Current Status

### ✅ Implemented
- Main window with two buttons (Patch / Create SEMPQ)
- Patch wizard structure (3 pages) with sidebar image
- SEMPQ wizard structure (2 pages) with sidebar image
- Plugin page with loading and configuration
- Cross-compilation toolchain files
- Wizard sidebar image (wizard.png)
- Add error handling

### 🚧 TODO
- Connect to actual patcher DLL
- Implement SEMPQ creation
- Add application icons (MPQDraft.ico, game icons, etc.)
- Load game registry entries
- Implement progress dialogs
- Testing on Windows

## Development Notes

### Plugin System Integration

The Qt GUI interfaces with the existing Windows plugin system using `QWidget::winId()`:

```cpp
HWND hwnd = (HWND)widget->winId();
plugin->Configure(hwnd);
```

This allows plugins to create native Windows dialogs as children of Qt windows.

### Why Qt6?

- Modern C++ support (C++17/20)
- Active development and long-term support
- CMake-first design (better for cross-compilation)
- Better performance than Qt5
- Qt5 enters maintenance mode in 2025

### Why Separate from Main Build?

Keeping the Qt GUI separate allows:
- Development on Linux without MFC dependencies
- Independent testing and iteration
- Clean cutover when ready
- No risk to existing MFC GUI during development

