cmake_minimum_required(VERSION 3.19)

# Project
project(
    qschematic
    VERSION 1.4.0
    LANGUAGES CXX
    HOMEPAGE_URL https://github.com/simulton/qschematic
)

# User options
option(QSCHEMATIC_BUILD_DEMO "Whether to build the demo project" ON)
option(QSCHEMATIC_DEPENDENCY_GPDS_DOWNLOAD "Whether to pull the GPDS dependency via FetchContent" ON)

# User settings
set(QSCHEMATIC_DEPENDENCY_GPDS_TARGET "gpds::gpds-static" CACHE STRING "The CMake target of the GPDS library to use")
set(CMAKE_DEBUG_POSTFIX d)

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
message(STATUS "  Build")
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
