# Project settings
project(qschematic
    VERSION 0.1.0
    LANGUAGES CXX)

# CMake settings
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

# List of source files
set(SOURCES_PRIVATE
    commands/commandbase.cpp
    commands/commanditemadd.cpp
    commands/commanditemmove.cpp
    commands/commanditemremove.cpp
    commands/commanditemvisibility.cpp
    commands/commandlabelrename.cpp
    commands/commandnoderesize.cpp
    commands/commandnoderotate.cpp
    commands/commandwirenetrename.cpp
    commands/commandwirepointmove.cpp
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
set(HEADERS_PUBLIC
    commands/commandbase.h
    commands/commanditemadd.h
    commands/commanditemmove.h
    commands/commanditemremove.h
    commands/commanditemvisibility.h
    commands/commandlabelrename.h
    commands/commandnoderesize.h
    commands/commandnoderotate.h
    commands/commands.h
    commands/commandwirenetrename.h
    commands/commandwirepointmove.h
    items/itemfunctions.h
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
    utils/itemscontainerutils.h
    utils/itemscustodian.h
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

# Include GPDS package
find_package(gpds 0.1.0 REQUIRED)



################################################################################
# Object library                                                               #
################################################################################

add_library(qschematic-objs OBJECT "")
target_compile_features(qschematic-objs
    PUBLIC
        cxx_std_17)
target_sources(qschematic-objs
    PRIVATE
        ${SOURCES_PRIVATE})
target_include_directories(qschematic-objs
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    INTERFACE
        $<INSTALL_INTERFACE:include>)
target_link_libraries(qschematic-objs Qt5::Core Qt5::Gui Qt5::Widgets)
target_link_libraries(qschematic-objs gpds::gpds-shared)


################################################################################
# Shared library                                                               #
################################################################################

add_library(qschematic-shared SHARED)
target_link_libraries(qschematic-shared qschematic-objs)


################################################################################
# Static library                                                               #
################################################################################

add_library(qschematic-static STATIC)
target_compile_definitions(
    qschematic-static
    PUBLIC
        QSCHEMATIC_STATIC_DEFINE
)
target_link_libraries(qschematic-static qschematic-objs)


################################################################################
# Common library properties                                                    #
################################################################################

set_target_properties(
    qschematic-objs
    qschematic-static
    qschematic-shared
    PROPERTIES
        OUTPUT_NAME "qschematic"
        ARCHIVE_OUTPUT_NAME "qschematic"
        VERSION ${PROJECT_VERSION}
        POSITION_INDEPENDENT_CODE 1
)



################################################################################
# Export header                                                                #
################################################################################

include(GenerateExportHeader)
generate_export_header(qschematic-shared
    BASE_NAME qschematic
    DEPRECATED_MACRO_NAME "QSCHEMATIC_DEPRECATED"
    NO_DEPRECATED_MACRO_NAME "QSCHEMATIC_NO_DEPRECATED"
    EXPORT_MACRO_NAME "QSCHEMATIC_EXPORT"
    NO_EXPORT_MACRO_NAME "QSCHEMATIC_NO_EXPORT"
    STATIC_DEFINE "QSCHEMATIC_STATIC_DEFINE"
    DEFINE_NO_DEPRECATED
)



################################################################################
# Install                                                                      #
################################################################################

# Some basic stuff we'll need in this section
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
set(ConfigPackageLocation ${CMAKE_INSTALL_LIBDIR}/cmake/QSchematic)

# Install headers
foreach( file ${HEADERS_PUBLIC} )
    get_filename_component( dir ${file} DIRECTORY )
    install( FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qschematic/${dir} )
endforeach()

# Install export header
install(
    FILES ${PROJECT_BINARY_DIR}/qschematic_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qschematic
)

# Targets
install(
    TARGETS
        qschematic-shared
        qschematic-static
        qschematic-objs
    EXPORT QSchematicTargets
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT lib
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT lib
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT bin
    INCLUDES
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qschematic
)

# Package version
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/QSchematic/QSchematicConfigVersion.cmake"
    VERSION ${QSchematic_VERSION}
    COMPATIBILITY AnyNewerVersion
)
install(
    FILES
        QSchematicConfig.cmake
        "${CMAKE_CURRENT_BINARY_DIR}/QSchematic/QSchematicConfigVersion.cmake"
    DESTINATION ${ConfigPackageLocation}
)

# Package configuration
configure_file(QSchematicConfig.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/QSchematic/QSchematicConfig.cmake"
    COPYONLY
)

# Package export targets
export(
    EXPORT QSchematicTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/QSchematic/QSchematicTargets.cmake"
    NAMESPACE QSchematic::
)
install(
    EXPORT QSchematicTargets
    FILE QSchematicTargets.cmake
    NAMESPACE QSchematic::
    DESTINATION ${ConfigPackageLocation}
)
