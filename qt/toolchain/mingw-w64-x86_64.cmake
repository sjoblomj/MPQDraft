# CMake toolchain file for cross-compiling from Linux to Windows (64-bit)
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=toolchain/mingw-w64-x86_64.cmake ..

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Specify the cross compiler
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# Where to look for the target environment
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

# Adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Qt6 for MinGW (if installed via package manager or Qt installer)
# You may need to adjust this path based on your Qt6 installation
# Example paths:
#   - Ubuntu package: /usr/lib/x86_64-linux-gnu/qt6
#   - Qt installer: ~/Qt/6.x.x/mingw_64
# set(Qt6_DIR "/path/to/qt6/lib/cmake/Qt6")

# Windows-specific flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

