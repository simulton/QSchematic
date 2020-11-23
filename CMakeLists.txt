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

################################################################################
# CPack                                                                        #
################################################################################
set(CPACK_PACKAGE_VENDOR "Simulton GmbH")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A library that allows creating diagrams such as flowcharts or even proper engineering schematics within a Qt application.")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/license.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
include(CPack)

