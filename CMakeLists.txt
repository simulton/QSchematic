cmake_minimum_required(VERSION 3.12)

project(qschematic)

# User options
option(BUILD_DEMO "Whether to build the demo project" ON)

# User settings
set(CMAKE_DEBUG_POSTFIX d)

# C++ options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt5 stuff
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Include the library
add_subdirectory(qschematic)

# Include the example(s)
if ( BUILD_DEMO )
    add_subdirectory(demo)
endif()
