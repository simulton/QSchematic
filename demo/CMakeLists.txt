set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

find_package(Qt5
    REQUIRED
    COMPONENTS
        Core
        Gui
        Widgets
        PrintSupport)

find_package(Gpds 0.1.0
    REQUIRED)

# Header files
set(HEADERS_PRIVATE
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
)

# Source files
set(SOURCES_PRIVATE
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
)

# Resources files
set(RESOURCES
    resources/icons/icons.qrc
)

# Compile executable
add_executable(qschematic-demo)
target_sources(qschematic-demo
    PRIVATE
        ${HEADERS_PRIVATE}
        ${SOURCES_PRIVATE}
        ${RESOURCES}
)
target_link_libraries(qschematic-demo Qt5::Core Qt5::Widgets Qt5::Gui Qt5::PrintSupport)
target_link_libraries(qschematic-demo qschematic-static)
target_link_libraries(qschematic-demo Gpds::gpds-static)
