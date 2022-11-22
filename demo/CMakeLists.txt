# Pull in external dependencies
include(../qschematic/external.cmake)

# Qt
find_package(
    Qt6
    COMPONENTS
        PrintSupport
)
if (NOT Qt6_FOUND)
    find_package(
        Qt5
        REQUIRED
        COMPONENTS
            PrintSupport
    )
endif()

# Source files
set(SOURCES_PRIVATE
    main.cpp
    mainwindow.cpp
)

# Resources files
set(RESOURCES
    resources/examples/examples.qrc
    resources/icons/icons.qrc
)

# Define a target name
set(TARGET qschematic-demo)

# Create executable
add_executable(${TARGET})

add_subdirectory(commands)
add_subdirectory(common)
add_subdirectory(items)
add_subdirectory(library)
add_subdirectory(netlist)

target_sources(
    ${TARGET}
    PRIVATE
        ${RESOURCES}
        main.cpp
        mainwindow.cpp
        mainwindow.h
)
target_link_libraries(
    ${TARGET}
    PRIVATE
        qschematic-static
        Qt::PrintSupport
)
set_target_properties(
    ${TARGET}
    PROPERTIES
        AUTOMOC ON
        AUTORCC ON
)

# Set window settings
# Usage: Add -DWINDOW_{dimension}={size} to the cmake options
if (WINDOW_WIDTH)
    add_compile_definitions(WINDOW_WIDTH=${WINDOW_WIDTH})
endif()
if (WINDOW_HEIGHT)
    add_compile_definitions(WINDOW_HEIGHT=${WINDOW_HEIGHT})
endif()
# Usage: Add -DWINDOW_MAXIMIZED=ON to the cmake options
if (WINDOW_MAXIMIZED)
    add_compile_definitions(WINDOW_MAXIMIZE)
endif()
