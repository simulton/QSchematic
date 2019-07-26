include_directories(
    .
    3rdparty/gpds/src
)

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

# List of source and header files
set(FILES ${SOURCES} ${HEADERS})

find_package(Qt5
        COMPONENTS
        Core
        Gui
        Widgets)

# Generate objects library
add_library(qschematic-objs OBJECT ${FILES})
# this is only needed for older compilers but doesn't hurt either to have it
set_target_properties(qschematic-objs
    PROPERTIES
        POSITION_INDEPENDENT_CODE 1)

qt5_use_modules(qschematic-objs Core Widgets Gui)

# Generate shared library
add_library(qschematic-shared SHARED $<TARGET_OBJECTS:qschematic-objs>)
set_target_properties(qschematic-shared
    PROPERTIES
        OUTPUT_NAME "qschematic")

# Generate static library
add_library(qschematic-static STATIC $<TARGET_OBJECTS:qschematic-objs>)
set_target_properties(qschematic-static
    PROPERTIES
        OUTPUT_NAME "qschematic")
