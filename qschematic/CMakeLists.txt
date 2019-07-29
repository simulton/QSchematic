# Project settings
project(qschematic
    VERSION 0.1.0)

# CMake settings
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

# List of source files
set(SOURCES
    commands/commanditemadd.cpp
    commands/commanditemmove.cpp
    commands/commanditemremove.cpp
    commands/commanditemvisibility.cpp
    commands/commandlabelrename.cpp
    commands/commandnoderesize.cpp
    commands/commandnoderotate.cpp
    items/connector.cpp
    items/item.cpp
    items/itemfactory.cpp
    items/itemmimedata.cpp
    items/label.cpp
    items/line.cpp
    items/node.cpp
    items/splinewire.cpp
    items/wire.cpp
    items/wirenet.cpp
    items/wirepoint.cpp
    items/wireroundedcorners.cpp
    scene.cpp
    settings.cpp
    utils.cpp
    view.cpp
)

# List of header files
set(HEADERS
    commands/commanditemadd.h
    commands/commanditemmove.h
    commands/commanditemremove.h
    commands/commanditemvisibility.h
    commands/commandlabelrename.h
    commands/commandnoderesize.h
    commands/commandnoderotate.h
    commands/commands.h
    items/connector.h
    items/item.h
    items/itemfactory.h
    items/itemmimedata.h
    items/label.h
    items/line.h
    items/node.h
    items/splinewire.h
    items/wire.h
    items/wirenet.h
    items/wirepoint.h
    items/wireroundedcorners.h
    netlist.h
    netlistgenerator.h
    scene.h
    settings.h
    types.h
    utils.h
    view.h
)

# Include Qt5 packages
find_package(Qt5
    REQUIRED
    COMPONENTS
        Core
        Gui
        Widgets)

# Include GPDS packages
find_package(Gpds 0.1.0 REQUIRED)

# Generate static library
add_library(qschematic-static STATIC ${HEADERS} ${SOURCES})
set_target_properties(qschematic-static
    PROPERTIES
        OUTPUT_NAME "qschematic"
        POSITION_INDEPENDENT_CODE 1)
target_link_libraries(qschematic-static Qt5::Core Qt5::Widgets Qt5::Gui)
target_link_libraries(qschematic-static Gpds::gpds-static)
