# CMake toolchain file for cross-compiling from Linux to Windows (32-bit)
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=toolchain/mingw-w64-i686.cmake ..

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR i686)

# Specify the cross compiler
set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)

# Where to look for the target environment
set(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32)

# Adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Qt6 for MinGW - REQUIRED for cross-compilation
# Install Qt6 with MinGW from: https://www.qt.io/download-qt-installer
# Then uncomment and adjust the path below to match your installation:
# set(Qt6_DIR "$ENV{HOME}/Qt/6.8.0/mingw_32/lib/cmake/Qt6")
#
# Or set it via environment variable:
# export Qt6_DIR=~/Qt/6.8.0/mingw_32/lib/cmake/Qt6

# Windows-specific flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

