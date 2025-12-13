#!/bin/bash
#
# Cross-compile MPQDraft for 32-bit Windows using MinGW-w64
#
# Prerequisites:
#   - MinGW-w64 i686 toolchain (apt install mingw-w64)
#   - Qt5 or Qt6 for MinGW (via MXE or manual installation)
#
# Usage:
#   ./tools/build-mingw32.sh [clean]
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

# Check for Qt
# First, try to find MXE Qt
MXE_ROOT="${MXE_ROOT:-/opt/mxe}"
if [ -d "$MXE_ROOT/usr/i686-w64-mingw32.static/qt5" ]; then
    QT_PREFIX="$MXE_ROOT/usr/i686-w64-mingw32.static/qt5"
    echo -e "${GREEN}Found MXE Qt5 at: $QT_PREFIX${NC}"
elif [ -d "$MXE_ROOT/usr/i686-w64-mingw32.shared/qt5" ]; then
    QT_PREFIX="$MXE_ROOT/usr/i686-w64-mingw32.shared/qt5"
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

cmake "${CMAKE_ARGS[@]}" "$PROJECT_ROOT/src"

# Build
echo ""
echo -e "${GREEN}Building...${NC}"
cmake --build . --parallel $(nproc)

echo ""
echo -e "${GREEN}=== Build Complete ===${NC}"
echo ""
echo "Output files:"
if [ -f "MPQDraftDLL.dll" ]; then
    echo "  - $BUILD_DIR/MPQDraftDLL.dll"
fi
if [ -f "MPQStub.exe" ]; then
    echo "  - $BUILD_DIR/MPQStub.exe"
fi
if [ -f "MPQDraftQt.exe" ]; then
    echo "  - $BUILD_DIR/MPQDraftQt.exe"
fi
echo ""

