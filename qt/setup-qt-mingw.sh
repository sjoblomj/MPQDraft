#!/bin/bash
# Helper script to set up Qt6 MinGW paths for cross-compilation

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Qt6 MinGW Setup Helper ===${NC}"
echo ""

# Common Qt installation locations
QT_LOCATIONS=(
    "$HOME/Qt"
    "/opt/Qt"
    "$HOME/.local/Qt"
)

# Find Qt installations
echo "Searching for Qt installations..."
FOUND_QT=()

for base in "${QT_LOCATIONS[@]}"; do
    if [ -d "$base" ]; then
        # Look for Qt versions
        for version_dir in "$base"/*/; do
            if [ -d "$version_dir" ]; then
                # Look for MinGW builds
                for mingw_dir in "$version_dir"mingw*/; do
                    if [ -d "$mingw_dir" ] && [ -f "$mingw_dir/lib/cmake/Qt6/Qt6Config.cmake" ]; then
                        FOUND_QT+=("$mingw_dir")
                    fi
                done
            fi
        done
    fi
done

if [ ${#FOUND_QT[@]} -eq 0 ]; then
    echo -e "${RED}No Qt6 MinGW installations found!${NC}"
    echo ""
    echo "You need to install Qt6 with MinGW support."
    echo ""
    echo -e "${YELLOW}Option 1: Use Qt Online Installer (Recommended)${NC}"
    echo "  1. Download: https://www.qt.io/download-qt-installer"
    echo "  2. Run the installer"
    echo "  3. Select Qt 6.x.x"
    echo "  4. Check 'MinGW 11.2.0 32-bit' and/or 'MinGW 11.2.0 64-bit'"
    echo "  5. Install"
    echo "  6. Run this script again"
    echo ""
    echo -e "${YELLOW}Option 2: Build for Linux only (for now)${NC}"
    echo "  ./build.sh linux"
    echo "  (You can develop the GUI on Linux and cross-compile later)"
    echo ""
    exit 1
fi

echo -e "${GREEN}Found Qt6 MinGW installations:${NC}"
for i in "${!FOUND_QT[@]}"; do
    echo "  [$i] ${FOUND_QT[$i]}"
done
echo ""

# If only one found, use it automatically
if [ ${#FOUND_QT[@]} -eq 1 ]; then
    SELECTED=0
    echo -e "${GREEN}Using: ${FOUND_QT[$SELECTED]}${NC}"
else
    # Ask user to select
    echo -n "Select installation [0-$((${#FOUND_QT[@]}-1))]: "
    read SELECTED
    
    if ! [[ "$SELECTED" =~ ^[0-9]+$ ]] || [ "$SELECTED" -ge ${#FOUND_QT[@]} ]; then
        echo -e "${RED}Invalid selection${NC}"
        exit 1
    fi
fi

QT_PATH="${FOUND_QT[$SELECTED]}"
QT_CMAKE_DIR="$QT_PATH/lib/cmake/Qt6"

echo ""
echo -e "${GREEN}Selected Qt6 installation:${NC}"
echo "  Path: $QT_PATH"
echo "  CMake: $QT_CMAKE_DIR"
echo ""

# Determine if it's 32-bit or 64-bit
if [[ "$QT_PATH" == *"mingw_32"* ]] || [[ "$QT_PATH" == *"mingw81_32"* ]]; then
    ARCH="32"
    TOOLCHAIN_FILE="toolchain/mingw-w64-i686.cmake"
elif [[ "$QT_PATH" == *"mingw_64"* ]] || [[ "$QT_PATH" == *"mingw81_64"* ]]; then
    ARCH="64"
    TOOLCHAIN_FILE="toolchain/mingw-w64-x86_64.cmake"
else
    echo -e "${YELLOW}Warning: Could not determine architecture from path${NC}"
    echo -n "Is this 32-bit or 64-bit? [32/64]: "
    read ARCH
    if [ "$ARCH" = "32" ]; then
        TOOLCHAIN_FILE="toolchain/mingw-w64-i686.cmake"
    else
        TOOLCHAIN_FILE="toolchain/mingw-w64-x86_64.cmake"
    fi
fi

echo -e "${BLUE}Updating $TOOLCHAIN_FILE...${NC}"

# Update the toolchain file
if grep -q "^set(Qt6_DIR" "$TOOLCHAIN_FILE"; then
    # Already has a set command, update it
    sed -i "s|^set(Qt6_DIR.*|set(Qt6_DIR \"$QT_CMAKE_DIR\")|" "$TOOLCHAIN_FILE"
else
    # Need to uncomment or add it
    if grep -q "^# set(Qt6_DIR" "$TOOLCHAIN_FILE"; then
        # Uncomment and update
        sed -i "s|^# set(Qt6_DIR.*|set(Qt6_DIR \"$QT_CMAKE_DIR\")|" "$TOOLCHAIN_FILE"
    else
        # Add it after the comment block
        sed -i "/^# Or set it via environment variable:/a set(Qt6_DIR \"$QT_CMAKE_DIR\")" "$TOOLCHAIN_FILE"
    fi
fi

echo -e "${GREEN}✓ Toolchain file updated!${NC}"
echo ""
echo -e "${BLUE}You can now build with:${NC}"
if [ "$ARCH" = "32" ]; then
    echo "  ./build.sh win32"
else
    echo "  ./build.sh win64"
fi
echo ""
echo -e "${YELLOW}Note: You can also set Qt6_DIR as an environment variable:${NC}"
echo "  export Qt6_DIR=\"$QT_CMAKE_DIR\""
echo "  ./build.sh win${ARCH}"

