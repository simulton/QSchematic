cmake_minimum_required(VERSION 3.16)

# Policies
if (POLICY CMP0076)
    cmake_policy(SET CMP0076 NEW)
endif()

# Project
project(qschematic)

# User options
option(BUILD_DEMO "Whether to build the demo project" ON)
option(DEPENDENCY_GPDS_FROM_SYSTEM "Whether to use the system's installation of GPDS or to pull GPDS internally" OFF)
option(DEPENDENCY_GPDS_SHARED "Controls whether to link to GPDS as a shared library or statically" ON)

# User settings
set(CMAKE_DEBUG_POSTFIX d)

# C++ options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Determine which GPDS target to use
set(DEPENDENCY_GPDS_TARGET gpds::gpds-static)
if (DEPENDENCY_GPDS_FROM_SYSTEM AND DEPENDENCY_GPDS_SHARED)
    set(DEPENDENCY_GPDS_TARGET gpds::gpds-shared)
endif()

# Include the library
add_subdirectory(qschematic)

# Include the example(s)
if (BUILD_DEMO)
    add_subdirectory(demo EXCLUDE_FROM_ALL)
endif()
