# Pull in external dependencies
include(../qschematic/external.cmake)

# Qt
find_package(
    Qt6
    COMPONENTS
        PrintSupport
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
add_subdirectory(resources)

target_sources(
    ${TARGET}
    PRIVATE
        main.cpp
        mainwindow.cpp
        mainwindow.hpp
)
target_link_libraries(
    ${TARGET}
    PRIVATE
        ${QSCHEMATIC_TARGET_INTERNAL}
        Qt::PrintSupport
)
set_target_properties(
    ${TARGET}
    PROPERTIES
        AUTOMOC ON
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
