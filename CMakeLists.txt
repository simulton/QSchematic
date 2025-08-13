cmake_minimum_required(VERSION 3.23)

# Project
project(
    qschematic
    VERSION 3.0.3
    LANGUAGES CXX
    HOMEPAGE_URL https://github.com/simulton/qschematic
)

# Enable CTest
enable_testing()

# User options
set(OPTION_BUILD_SHARED_DEFAULT ON)
if (MSVC)
    set(OPTION_BUILD_SHARED_DEFAULT OFF)
endif()
option(QSCHEMATIC_BUILD_STATIC "Whether to build a static library" ON)
option(QSCHEMATIC_BUILD_SHARED "Whether to build a shared library" ${OPTION_BUILD_SHARED_DEFAULT})
option(QSCHEMATIC_BUILD_DEMO "Whether to build the demo project" ON)
option(QSCHEMATIC_DEPENDENCY_GPDS_DOWNLOAD "Whether to pull the GPDS dependency via FetchContent" ON)

# User settings
set(QSCHEMATIC_DEPENDENCY_GPDS_TARGET "gpds::gpds-static" CACHE STRING "The CMake target of the GPDS library to use")
set(CMAKE_DEBUG_POSTFIX d)

# When using MSVC, only allow enabling either static or shared libs but not both to prevent name conflicts on the resulting *.LIB targets
if (MSVC)
    if (QSCHEMATIC_BUILD_STATIC AND QSCHEMATIC_BUILD_SHARED)
        message(FATAL_ERROR "QSchematic: Can only build either the static or the shared library but not both at the same time when using MSVC.")
    endif()
endif()

# Library target for internal usage (i.e. for demos, examples, tests, ...)
set(QSCHEMATIC_TARGET_INTERNAL qschematic-static)
if (NOT QSCHEMATIC_BUILD_STATIC)
    set(QSCHEMATIC_TARGET_INTERNAL qschematic-shared)
endif()

# Project requirements
set(QSCHEMATIC_DEPENDENCY_GPDS_MINIMUM_VERSION  "1.8.4")
set(QSCHEMATIC_DEPENDENCY_GPDS_DOWNLOAD_VERSION "1.10.0")

# Include the library
add_subdirectory(qschematic)

# Include the example(s)
if (QSCHEMATIC_BUILD_DEMO)
    add_subdirectory(demo)
endif()

# Print options
message(STATUS "")
message(STATUS "-------------------------")
message(STATUS "QSchematic configuration:")
message(STATUS "  version      : " ${PROJECT_VERSION})
message(STATUS "")
message(STATUS "  Build")
message(STATUS "    Static     : " ${QSCHEMATIC_BUILD_STATIC})
message(STATUS "    Shared     : " ${QSCHEMATIC_BUILD_SHARED})
message(STATUS "    Demo       : " ${QSCHEMATIC_BUILD_DEMO})
message(STATUS "")
message(STATUS "  Dependencies")
message(STATUS "    GPDS")
message(STATUS "      Download : " ${QSCHEMATIC_DEPENDENCY_GPDS_DOWNLOAD})
message(STATUS "      Target   : " ${QSCHEMATIC_DEPENDENCY_GPDS_TARGET})
message(STATUS "-------------------------")
message(STATUS "")

################################################################################
# CPack                                                                        #
################################################################################
set(CPACK_PACKAGE_VENDOR "Simulton GmbH")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A library for diagrams such as flowcharts or even proper engineering schematics within a Qt application.")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/license.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
include(CPack)
