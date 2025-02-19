include(FetchContent)

########################################################################################################################
# GPDS
########################################################################################################################
if (QSCHEMATIC_DEPENDENCY_GPDS_DOWNLOAD)
    set(GPDS_BUILD_TESTS    OFF CACHE INTERNAL "")
    set(GPDS_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(GPDS_FEATURE_SPDLOG OFF CACHE INTERNAL "")

    FetchContent_Declare(
        gpds
        GIT_REPOSITORY https://github.com/simulton/gpds
        GIT_TAG        ${QSCHEMATIC_DEPENDENCY_GPDS_DOWNLOAD_VERSION}
    )
    FetchContent_MakeAvailable(gpds)
else()
    find_package(
        gpds
        ${QSCHEMATIC_DEPENDENCY_GPDS_MINIMUM_VERSION}
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
