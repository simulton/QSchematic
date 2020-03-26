include(FetchContent)

# GPDS
if (GPDS_DEPENDENCY_FROM_SYSTEM)
    find_package(gpds 0.1.0 REQUIRED)
else()
    FetchContent_Declare(
        gpds
        GIT_REPOSITORY https://github.com/simulton/gpds
        GIT_TAG        develop
    )

    FetchContent_GetProperties(gpds)
    if(NOT gpds_POPULATED)
        FetchContent_Populate(gpds)
        add_subdirectory(${gpds_SOURCE_DIR} ${gpds_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()

    # Create alias to allow uniform access through the namespace
    add_library(gpds::gpds-objs ALIAS gpds-objs)
    add_library(gpds::gpds-static ALIAS gpds-static)
    add_library(gpds::gpds-shared ALIAS gpds-shared)
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
