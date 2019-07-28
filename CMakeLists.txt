cmake_minimum_required(VERSION 3.12)

project(qschematic)

# User options
option(QSCHEMATIC_BUILD_DEMO "Whether to build the demo project" on)

# C++ options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt options
set(Qt5_DIR "C:/Qt/5.13.0/mingw73_64/lib/cmake/Qt5")

# Qt5 stuff
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Include the library
add_subdirectory(qschematic)

# Include the example(s)
if ( QSCHEMATIC_BUILD_DEMO )
    add_subdirectory(demo)
endif()
