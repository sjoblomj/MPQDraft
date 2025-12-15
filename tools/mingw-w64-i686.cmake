# CMake Toolchain File for cross-compiling to 32-bit Windows using MinGW-w64
#
# Usage:
#   cmake -DCMAKE_TOOLCHAIN_FILE=../tools/mingw-w64-i686.cmake ..
#
# This toolchain file configures CMake to use the i686-w64-mingw32 compiler
# to produce 32-bit Windows executables from Linux.

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR i686)

# Specify the cross-compiler
# Use the -posix variant for better C++11/14/17 threading support
set(CMAKE_C_COMPILER i686-w64-mingw32-gcc-posix)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++-posix)
set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)

# Where to look for the target environment
set(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32)

# Adjust the default behavior of the FIND_XXX() commands:
# - Search for programs in the host environment
# - Search for libraries and headers in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Set MINGW flag for CMakeLists.txt conditionals
set(MINGW TRUE)

# Ensure we're building for 32-bit
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")

# Static linking of libgcc and libstdc++ to avoid runtime dependencies
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++")

# Windows-specific definitions
add_definitions(-DWIN32 -D_WIN32 -D__WIN32__ -DWINVER=0x0601 -D_WIN32_WINNT=0x0601)

# Resource compiler flags - define _WIN32 for windres so .rc files compile correctly
set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} -D_WIN32")
