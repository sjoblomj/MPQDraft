#!/bin/bash
#
# Cross-compile MPQDraft for 32-bit Windows using MinGW-w64
#
# Prerequisites:
#   - MinGW-w64 i686 toolchain:
#       sudo apt install mingw-w64 g++-mingw-w64-i686-posix
#
#   - (Optional) Qt5 for MinGW via MXE for GUI support:
#       See instructions below or in README.md
#
# Usage:
#   ./tools/build-mingw32.sh [clean]
#
# Environment variables:
#   MXE_ROOT         - Path to MXE installation (default: /opt/mxe)
#   QT_MINGW32_PREFIX - Path to Qt installation for MinGW
#
# Without Qt, only MPQDraftDLL.dll and MPQStub.exe will be built.
# With Qt (via MXE), the full GUI application can also be built.
#
# === Setting up MXE for Qt5 cross-compilation ===
#
# MXE (M Cross Environment) provides Qt5 pre-configured for MinGW cross-compilation.
# This is the recommended way to build the Qt GUI from Linux.
#
# 1. Install MXE dependencies (Ubuntu/Debian):
#
#    sudo apt install autoconf automake autopoint bash bison bzip2 flex \
#        g++ g++-multilib gettext git gperf intltool libc6-dev-i386 \
#        libgdk-pixbuf2.0-dev libltdl-dev libgl-dev libpcre3-dev libssl-dev \
#        libtool-bin libxml-parser-perl lzip make openssl p7zip-full patch \
#        perl python3 python3-mako python3-packaging python-is-python3 ruby \
#        sed unzip wget xz-utils
#
# 2. Clone and build MXE with Qt5 (takes 1-2 hours):
#
#    git clone https://github.com/mxe/mxe.git /opt/mxe
#    cd /opt/mxe
#    make MXE_TARGETS='i686-w64-mingw32.static' qt5
#
# 3. Build MPQDraft with Qt:
#
#    MXE_ROOT=/opt/mxe ./tools/build-mingw32.sh
#
# The resulting executables will be statically linked and won't require
# Qt DLLs to be distributed alongside the application.
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build-mingw32"
TOOLCHAIN_FILE="$SCRIPT_DIR/mingw-w64-i686.cmake"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== MPQDraft MinGW-w64 32-bit Cross-Compilation ===${NC}"
echo ""

# Check for clean argument
if [ "$1" = "clean" ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "$BUILD_DIR"
    echo -e "${GREEN}Done.${NC}"
    exit 0
fi

# Check for MinGW-w64 compiler
if ! command -v i686-w64-mingw32-g++-posix &> /dev/null; then
    echo -e "${RED}Error: i686-w64-mingw32-g++-posix not found.${NC}"
    echo "Please install MinGW-w64:"
    echo "  sudo apt install mingw-w64 g++-mingw-w64-i686-posix"
    exit 1
fi

echo "Using compiler: $(i686-w64-mingw32-g++-posix --version | head -1)"
echo ""

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Check for Qt and MXE
# First, try to find MXE Qt
MXE_ROOT="${MXE_ROOT:-/opt/mxe}"
USE_MXE=false
MXE_TARGET=""

if [ -d "$MXE_ROOT/usr/i686-w64-mingw32.static/qt5" ]; then
    QT_PREFIX="$MXE_ROOT/usr/i686-w64-mingw32.static/qt5"
    MXE_TARGET="i686-w64-mingw32.static"
    USE_MXE=true
    echo -e "${GREEN}Found MXE Qt5 at: $QT_PREFIX${NC}"
elif [ -d "$MXE_ROOT/usr/i686-w64-mingw32.shared/qt5" ]; then
    QT_PREFIX="$MXE_ROOT/usr/i686-w64-mingw32.shared/qt5"
    MXE_TARGET="i686-w64-mingw32.shared"
    USE_MXE=true
    echo -e "${GREEN}Found MXE Qt5 at: $QT_PREFIX${NC}"
elif [ -n "$QT_MINGW32_PREFIX" ]; then
    QT_PREFIX="$QT_MINGW32_PREFIX"
    echo -e "${GREEN}Using Qt from QT_MINGW32_PREFIX: $QT_PREFIX${NC}"
else
    echo -e "${YELLOW}Warning: Qt for MinGW not found.${NC}"
    echo ""
    echo "To build with Qt GUI, you need Qt compiled for MinGW-w64 i686."
    echo "Options:"
    echo "  1. Install MXE and build Qt: https://mxe.cc/"
    echo "     make MXE_TARGETS='i686-w64-mingw32.static' qt5"
    echo ""
    echo "  2. Set QT_MINGW32_PREFIX to your Qt installation path"
    echo ""
    echo "Building without Qt (DLL and Stub only)..."
    echo ""
    QT_PREFIX=""
fi

# Configure with CMake
echo -e "${GREEN}Configuring with CMake...${NC}"

# When using MXE, we must use MXE's CMake wrapper which sets up the proper
# toolchain and environment. Using system CMake with MXE's Qt causes errors
# like "Unknown CMake command _qt5_Core_check_file_exists".
if [ "$USE_MXE" = true ] && [ -x "$MXE_ROOT/usr/bin/$MXE_TARGET-cmake" ]; then
    # Add MXE bin directory to PATH - required for ccache to find the compiler
    export PATH="$MXE_ROOT/usr/bin:$PATH"

    # MXE uses ccache by default. If MXE is installed system-wide (e.g., /opt/mxe),
    # ccache may try to write to a directory the user doesn't have access to.
    # Redirect ccache to the user's home directory.
    export CCACHE_DIR="${CCACHE_DIR:-$HOME/.cache/ccache}"
    mkdir -p "$CCACHE_DIR"

    CMAKE_CMD="$MXE_ROOT/usr/bin/$MXE_TARGET-cmake"
    echo "Using MXE CMake wrapper: $CMAKE_CMD"

    # MXE's cmake wrapper already sets up the toolchain, so we don't need to specify it
    CMAKE_ARGS=(
        -DCMAKE_BUILD_TYPE=Release
    )
else
    CMAKE_CMD="cmake"
    CMAKE_ARGS=(
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE"
        -DCMAKE_BUILD_TYPE=Release
    )

    if [ -n "$QT_PREFIX" ]; then
        CMAKE_ARGS+=(
            -DCMAKE_PREFIX_PATH="$QT_PREFIX"
            -DQt5_DIR="$QT_PREFIX/lib/cmake/Qt5"
        )
    fi
fi

"$CMAKE_CMD" "${CMAKE_ARGS[@]}" "$PROJECT_ROOT/src"

# Build
echo ""
echo -e "${GREEN}Building...${NC}"
"$CMAKE_CMD" --build . --parallel $(nproc)

echo ""
echo -e "${GREEN}=== Build Complete ===${NC}"
echo ""
echo "Output files (in $BUILD_DIR directory):"
if [ -f "$BUILD_DIR/MPQDraftDLL.dll" ]; then
    echo "  - $BUILD_DIR/MPQDraftDLL.dll"
fi
if [ -f "$BUILD_DIR/MPQStub.exe" ]; then
    echo "  - $BUILD_DIR/MPQStub.exe"
fi
# MPQDraft GUI exe is named MPQDraft-<date>.exe
MPQDRAFT_EXE=$(ls -t "$BUILD_DIR"/MPQDraft-*.exe 2>/dev/null | head -1)
if [ -n "$MPQDRAFT_EXE" ]; then
    echo "  - $MPQDRAFT_EXE"
fi
echo ""
