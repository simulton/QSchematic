include(FetchContent)

########################################################################################################################
# GPDS
########################################################################################################################
if (QSCHEMATIC_DEPENDENCY_GPDS_DOWNLOAD)
    FetchContent_Declare(
        gpds
        GIT_REPOSITORY https://github.com/simulton/gpds
        GIT_TAG        1.5.0
    )
    FetchContent_GetProperties(gpds)
    if(NOT gpds_POPULATED)
        FetchContent_Populate(gpds)
        set(GPDS_BUILD_TESTS    OFF CACHE INTERNAL "")
        set(GPDS_BUILD_EXAMPLES OFF CACHE INTERNAL "")
        set(GPDS_FEATURE_SPDLOG OFF CACHE INTERNAL "")
        add_subdirectory(${gpds_SOURCE_DIR} ${gpds_BINARY_DIR})

        # Create alias libraries
        add_library(gpds::gpds-static ALIAS gpds-static)
        add_library(gpds::gpds-shared ALIAS gpds-shared)
    endif()
else()
    find_package(
        gpds
        REQUIRED
    )
endif()


########################################################################################################################
# Qt
########################################################################################################################

# Try to find Qt6
find_package(
    Qt6
    COMPONENTS
        Core
        Gui
        Widgets
)

# If Qt6 was not found, fallback to Qt5
# Require minimum Qt 5.15 for versionless cmake targets. This can be relaxed down to Qt 5.6 (?) if needed by modifying
# The CMake target linking statements as demonstrated in the Qt documentation.
if (NOT Qt6_FOUND)
    find_package(
        Qt5 5.15
        REQUIRED
        COMPONENTS
            Core
            Gui
            Widgets
    )
endif()
