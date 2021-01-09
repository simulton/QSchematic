include(FetchContent)

# GPDS
if (DEPENDENCY_GPDS_FROM_SYSTEM)
    find_package(gpds 0.1.0 REQUIRED)

    set(DEPENDENCY_GPDS_TARGET "gpds::gpds-shared")
else()
    FetchContent_Declare(
        gpds
        GIT_REPOSITORY https://github.com/simulton/gpds
        GIT_TAG        develop
    )
    FetchContent_MakeAvailable(gpds)

    set(DEPENDENCY_GPDS_TARGET "gpds-shared")
endif()

# Qt5
find_package(
    Qt5
    REQUIRED
    COMPONENTS
        Core
        Gui
        Widgets
)
