# Include Qt5 packages
find_package(Qt5
    REQUIRED
    COMPONENTS
        Core
        Gui
        Widgets)

add_subdirectory(test)

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

add_library(wire_system)

target_compile_features(wire_system
    PUBLIC
        cxx_std_17)

target_sources(wire_system
    PRIVATE
        ${SOURCES_PRIVATE}
        ${HEADERS_PUBLIC})

target_link_libraries(wire_system Qt5::Core Qt5::Gui Qt5::Widgets)

install( FILES ${HEADERS_PUBLIC} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qschematic/wire_system )
