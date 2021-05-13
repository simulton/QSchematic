# Pull in external dependencies
include(../qschematic/external.cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

# Qt5
find_package(Qt5
    COMPONENTS
        PrintSupport
)

# Header files
set(HEADERS_PRIVATE
    common/treeitem.h
    common/treemodel.h
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
    netlistviewer/netlistviewer.h
    netlistviewer/netlistviewermodel.h
    netlistviewer/netlistviewerview.h
    mainwindow.h
)

# Source files
set(SOURCES_PRIVATE
    common/treeitem.cpp
    common/treemodel.cpp
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
    netlistviewer/netlistviewer.cpp
    netlistviewer/netlistviewermodel.cpp
    netlistviewer/netlistviewerview.cpp
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
target_include_directories(
    qschematic-demo
    PRIVATE
        ..
)
target_link_libraries(
    qschematic-demo
    PRIVATE
        qschematic-static
        Qt5::PrintSupport
)
set_target_properties(
    qschematic-demo
    PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        AUTOUIC ON
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
