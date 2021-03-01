include(FetchContent)

# GPDS
if (DEPENDENCY_GPDS_DOWNLOAD)
    FetchContent_Declare(
        gpds
        GIT_REPOSITORY https://github.com/simulton/gpds
        GIT_TAG        master
    )
    FetchContent_MakeAvailable(gpds)
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
