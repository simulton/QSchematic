# External dependencies
include(external.cmake)

# CMake settings
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Define directory base path for FILE_SET
set(QSCHEMATIC_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Add the wire system
add_subdirectory(wire_system)

# Setup target names
set(TARGET_BASE_NAME "qschematic")
set(TARGET_STATIC    ${TARGET_BASE_NAME}-static)
set(TARGET_SHARED    ${TARGET_BASE_NAME}-shared)

# This function sets stuff up common to all targets
function(setup_target_common target)
    target_compile_features(
        ${target}
        PUBLIC
            cxx_std_17
    )

    target_sources(
        ${target}
        PUBLIC
        FILE_SET
            HEADERS
            BASE_DIRS ${QSCHEMATIC_BASE_DIR}
            FILES
                commands/base.h
                commands/item_add.h
                commands/item_move.h
                commands/item_remove.h
                commands/item_visibility.h
                commands/label_rename.h
                commands/rectitem_resize.h
                commands/rectitem_rotate.h
                commands/commands.h
                commands/wirenet_rename.h
                commands/wirepoint_move.h
                items/itemfunctions.h
                items/connector.h
                items/item.h
                items/itemfactory.h
                items/itemmimedata.h
                items/label.h
                items/node.h
                items/rectitem.h
                items/splinewire.h
                items/widget.h
                items/wire.h
                items/wirenet.h
                items/wireroundedcorners.h
                utils/itemscontainerutils.h
                utils/itemscustodian.h
                wire_system/connectable.h
                wire_system/line.h
                wire_system/manager.h
                wire_system/wire.h
                wire_system/point.h
                wire_system/net.h
                netlist.h
                netlist_writer_json.h
                netlistgenerator.h
                scene.h
                settings.h
                types.h
                utils.h
                view.h

        PRIVATE
            commands/base.cpp
            commands/item_add.cpp
            commands/item_move.cpp
            commands/item_remove.cpp
            commands/item_visibility.cpp
            commands/label_rename.cpp
            commands/rectitem_resize.cpp
            commands/rectitem_rotate.cpp
            commands/wirenet_rename.cpp
            commands/wirepoint_move.cpp
            items/connector.cpp
            items/item.cpp
            items/itemfactory.cpp
            items/itemmimedata.cpp
            items/label.cpp
            items/node.cpp
            items/rectitem.cpp
            items/splinewire.cpp
            items/widget.cpp
            items/wire.cpp
            items/wirenet.cpp
            items/wireroundedcorners.cpp
            wire_system/line.cpp
            wire_system/manager.cpp
            wire_system/wire.cpp
            wire_system/point.cpp
            wire_system/net.cpp
            scene.cpp
            settings.cpp
            utils.cpp
            view.cpp
    )

    target_include_directories(
        ${target}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
            $<INSTALL_INTERFACE:..>
    )

    target_link_libraries(
        ${target}
        PUBLIC
            Qt::Core
            Qt::Gui
            Qt::Widgets
            ${QSCHEMATIC_DEPENDENCY_GPDS_TARGET}
    )

    set_target_properties(
        ${target}
        PROPERTIES
            AUTOMOC ON
            AUTORCC ON
    )

    set_target_properties(
        ${target}
        PROPERTIES
            OUTPUT_NAME "qschematic"
            ARCHIVE_OUTPUT_NAME "qschematic"
            VERSION ${PROJECT_VERSION}
            POSITION_INDEPENDENT_CODE 1
    )
endfunction()


################################################################################
# Shared library                                                               #
################################################################################

add_library(${TARGET_SHARED} SHARED)
setup_target_common(${TARGET_SHARED})


################################################################################
# Static library                                                               #
################################################################################

add_library(${TARGET_STATIC} STATIC)
setup_target_common(${TARGET_STATIC})

target_compile_definitions(
    ${TARGET_STATIC}
    PUBLIC
        QSCHEMATIC_STATIC_DEFINE
)


################################################################################
# Install                                                                      #
################################################################################

# Some basic stuff we'll need in this section
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
set(ConfigPackageLocation ${CMAKE_INSTALL_LIBDIR}/cmake/qschematic)

# Targets
install(
    TARGETS
        ${TARGET_STATIC}
        ${TARGET_SHARED}
    EXPORT qschematic-targets
    FILE_SET
        HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qschematic
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT lib
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT lib
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT bin
)

# Package version
write_basic_package_version_file(
    qschematic-config-version.cmake
    VERSION ${QSchematic_VERSION}
    COMPATIBILITY AnyNewerVersion
)
configure_file(qschematic-config.cmake.in qschematic-config.cmake @ONLY)
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/qschematic-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/qschematic-config-version.cmake
    DESTINATION ${ConfigPackageLocation}
)

# Package export targets
export(
    EXPORT qschematic-targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/qschematic/qschematic-targets.cmake"
    NAMESPACE qschematic::
)
install(
    EXPORT qschematic-targets
    FILE qschematic-targets.cmake
    NAMESPACE qschematic::
    DESTINATION ${ConfigPackageLocation}
)
