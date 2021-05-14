cmake_minimum_required(VERSION 3.19)

# Policies
if (POLICY CMP0076)
    cmake_policy(SET CMP0076 NEW)
endif()

# Project
project(qschematic)

# User options
option(QSCHEMATIC_BUILD_DEMO "Whether to build the demo project" ON)
option(QSCHEMATIC_DEPENDENCY_GPDS_DOWNLOAD "Whether to pull the GPDS dependency via FetchContent" ON)

# User settings
set(QSCHEMATIC_DEPENDENCY_GPDS_TARGET "gpds-static" CACHE STRING "The CMake target of the GPDS library to use")
set(CMAKE_DEBUG_POSTFIX d)

# Include the library
add_subdirectory(qschematic)

# Include the example(s)
if (QSCHEMATIC_BUILD_DEMO)
    add_subdirectory(demo EXCLUDE_FROM_ALL)
endif()

# Print options
message("")
message("-------------------------")
message("QSchematic configuration:")
message("  Build")
message("    Demo       : " ${QSCHEMATIC_BUILD_DEMO})
message("  Dependencies")
message("    GPDS")
message("      Download : " ${QSCHEMATIC_DEPENDENCY_GPDS_DOWNLOAD})
message("      Target   : " ${QSCHEMATIC_DEPENDENCY_GPDS_TARGET})
message("-------------------------")
message("")

################################################################################
# CPack                                                                        #
################################################################################
set(CPACK_PACKAGE_VENDOR "Simulton GmbH")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A library for diagrams such as flowcharts or even proper engineering schematics within a Qt application.")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/license.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
include(CPack)
