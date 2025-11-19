#!/bin/bash
# Build script for MPQDraft Qt GUI

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_usage() {
    echo "Usage: $0 [linux|win32|win64|clean]"
    echo ""
    echo "Options:"
    echo "  linux   - Build for Linux (native, for development/testing)"
    echo "  win32   - Cross-compile for Windows 32-bit"
    echo "  win64   - Cross-compile for Windows 64-bit"
    echo "  clean   - Remove all build directories"
    echo ""
    echo "Examples:"
    echo "  $0 linux          # Build for Linux"
    echo "  $0 win32          # Cross-compile for Windows 32-bit"
    echo "  $0 clean          # Clean all builds"
}

build_linux() {
    echo -e "${GREEN}Building for Linux...${NC}"
    mkdir -p build
    cd build
    cmake ..
    cmake --build . -j$(nproc)
    cd ..
    echo -e "${GREEN}✓ Build complete: build/MPQDraftQt${NC}"
    echo -e "${YELLOW}Run with: ./build/MPQDraftQt${NC}"
}

build_win32() {
    echo -e "${GREEN}Cross-compiling for Windows 32-bit...${NC}"
    
    # Check if MinGW is installed
    if ! command -v i686-w64-mingw32-g++ &> /dev/null; then
        echo -e "${RED}Error: MinGW-w64 (32-bit) not found${NC}"
        echo "Install with: sudo apt-get install g++-mingw-w64-i686"
        exit 1
    fi
    
    mkdir -p build-win32
    cd build-win32
    cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain/mingw-w64-i686.cmake ..
    cmake --build . -j$(nproc)
    cd ..
    echo -e "${GREEN}✓ Build complete: build-win32/MPQDraftQt.exe${NC}"
    echo -e "${YELLOW}Test with Wine: wine build-win32/MPQDraftQt.exe${NC}"
}

build_win64() {
    echo -e "${GREEN}Cross-compiling for Windows 64-bit...${NC}"
    
    # Check if MinGW is installed
    if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
        echo -e "${RED}Error: MinGW-w64 (64-bit) not found${NC}"
        echo "Install with: sudo apt-get install g++-mingw-w64-x86-64"
        exit 1
    fi
    
    mkdir -p build-win64
    cd build-win64
    cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain/mingw-w64-x86_64.cmake ..
    cmake --build . -j$(nproc)
    cd ..
    echo -e "${GREEN}✓ Build complete: build-win64/MPQDraftQt.exe${NC}"
    echo -e "${YELLOW}Test with Wine: wine64 build-win64/MPQDraftQt.exe${NC}"
}

clean_all() {
    echo -e "${YELLOW}Cleaning all build directories...${NC}"
    rm -rf build build-win32 build-win64
    echo -e "${GREEN}✓ Clean complete${NC}"
}

# Main script
if [ $# -eq 0 ]; then
    print_usage
    exit 1
fi

case "$1" in
    linux)
        build_linux
        ;;
    win32)
        build_win32
        ;;
    win64)
        build_win64
        ;;
    clean)
        clean_all
        ;;
    *)
        echo -e "${RED}Error: Unknown option '$1'${NC}"
        echo ""
        print_usage
        exit 1
        ;;
esac

