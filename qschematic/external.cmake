include(FetchContent)

########################################################################################################################
# GPDS
########################################################################################################################
if (QSCHEMATIC_DEPENDENCY_GPDS_DOWNLOAD)
    FetchContent_Declare(
        gpds
        GIT_REPOSITORY https://github.com/simulton/gpds
        GIT_TAG        ${QSCHEMATIC_DEPENDENCY_GPDS_DOWNLOAD_VERSION}
    )
    FetchContent_GetProperties(gpds)
    if(NOT gpds_POPULATED)
        FetchContent_Populate(gpds)
        set(GPDS_BUILD_TESTS    OFF CACHE INTERNAL "")
        set(GPDS_BUILD_EXAMPLES OFF CACHE INTERNAL "")
        set(GPDS_FEATURE_SPDLOG OFF CACHE INTERNAL "")
        add_subdirectory(${gpds_SOURCE_DIR} ${gpds_BINARY_DIR})
    endif()
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
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} 5.15 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR}
    OPTIONAL_COMPONENTS
    Network
    Sql
    Concurrent
    SerialPort
    Positioning
    Xml
    WebSockets
    Svg
    Multimedia
    MultimediaWidgets
    OpenGL
    Quick
    QuickWidgets
    Qml
    Charts
    PrintSupport
    )
