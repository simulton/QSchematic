include_directories(.)
include_directories(commands)
include_directories(items)
include_directories(itemslibrary)

find_package(Qt5
        COMPONENTS
        Core
        Gui
        Widgets)

# Header files
set(HEADERS
    commands/commandnodeaddconnector.h
    commands/commands.h
    items/customitemfactory.h
    items/fancywire.h
    items/flowend.h
    items/flowstart.h
    items/itemtypes.h
    items/operation.h
    items/operationconnector.h
    items/operationdemo1.h
    itemslibrary/iteminfo.h
    itemslibrary/itemslibrarymodel.h
    itemslibrary/itemslibrarymodelitem.h
    itemslibrary/itemslibraryview.h
    itemslibrary/itemsslibrarywidget.h
    mainwindow.h
    resources.h)

# Source files
set(SOURCES
    commands/commandnodeaddconnector.cpp
    items/customitemfactory.cpp
    items/fancywire.cpp
    items/flowend.cpp
    items/flowstart.cpp
    items/operation.cpp
    items/operationconnector.cpp
    items/operationdemo1.cpp
    itemslibrary/itemslibrarymodel.cpp
    itemslibrary/itemslibraryview.cpp
    itemslibrary/itemsslibrarywidget.cpp
    main.cpp
    mainwindow.cpp
    resources.cpp)

# Compile executable
add_executable(qschematic-demo ${HEADERS} ${SOURCES})
qt5_use_modules(qschematic-demo Widgets Gui)
target_link_libraries(qschematic-demo qschematic-static)
