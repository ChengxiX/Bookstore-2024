# Minimum CMake version required
cmake_minimum_required(VERSION 3.20)

# Project name and version
project(Bookstore VERSION 0.1)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(code src/main.cpp)

# Include directories
include_directories(include)

# Specify the directory where the generated files will be placed
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY /tmp/ramdisk/compiler_tmp/home/xie/Bookstore-2024)

# Link libraries
# target_link_libraries(Bookstore <library_name>)