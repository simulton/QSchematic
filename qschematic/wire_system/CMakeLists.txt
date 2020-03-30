# Include Qt5 packages
find_package(Qt5
    REQUIRED
    COMPONENTS
        Core
        Gui
)

add_subdirectory(test EXCLUDE_FROM_ALL)

set(SOURCES_PRIVATE
    line.cpp
    manager.cpp
    wire.cpp
    point.cpp
    net.cpp
    ../utils.cpp
    ../settings.cpp
)

set(HEADERS_PUBLIC
    line.h
    manager.h
    wire.h
    point.h
    net.h
    connectable.h
)

add_library(wire_system OBJECT)

target_compile_features(
    wire_system
    PUBLIC
        cxx_std_17
)

target_sources(
    wire_system
    PRIVATE
        ${SOURCES_PRIVATE}
        ${HEADERS_PUBLIC}
)

target_link_libraries(
    wire_system
    PUBLIC
        Qt5::Core
        Qt5::Gui
)

target_include_directories(
    wire_system
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
)

set_target_properties(
    wire_system
    PROPERTIES
        AUTOMOC ON
)
