# Minimum CMake version required
cmake_minimum_required(VERSION 3.20)

# Project name and version
project(Bookstore VERSION 0.1)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(Bookstore src/main.cpp include/ramdomdb.cpp include/kvdb.cpp include/seqdb.cpp)

# Include directories
include_directories(include)

# Link libraries
# target_link_libraries(Bookstore <library_name>)