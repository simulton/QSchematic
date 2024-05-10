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
            cxx_std_20
    )

    # add alias to support cmake superbuild pattern
    add_library(qschematic::${target} ALIAS ${target})

    target_sources(
        ${target}
        PUBLIC
        FILE_SET
            HEADERS
            BASE_DIRS ${QSCHEMATIC_BASE_DIR}
            FILES
                commands/base.hpp
                commands/item_add.hpp
                commands/item_move.hpp
                commands/item_remove.hpp
                commands/item_visibility.hpp
                commands/label_rename.hpp
                commands/rectitem_resize.hpp
                commands/rectitem_rotate.hpp
                commands/commands.hpp
                commands/wirenet_rename.hpp
                commands/wirepoint_move.hpp
                items/itemfunctions.hpp
                items/connector.hpp
                items/item.hpp
                items/itemfactory.hpp
                items/itemmimedata.hpp
                items/label.hpp
                items/node.hpp
                items/rectitem.hpp
                items/splinewire.hpp
                items/widget.hpp
                items/wire.hpp
                items/wirenet.hpp
                items/wireroundedcorners.hpp
                utils/itemscontainerutils.hpp
                utils/itemscustodian.hpp
                wire_system/connectable.hpp
                wire_system/line.hpp
                wire_system/manager.hpp
                wire_system/wire.hpp
                wire_system/point.hpp
                wire_system/net.hpp
                background.hpp
                netlist.hpp
                netlist_writer_json.hpp
                netlistgenerator.hpp
                scene.hpp
                settings.hpp
                types.hpp
                utils.hpp
                view.hpp

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
            background.cpp
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
            POSITION_INDEPENDENT_CODE ON
    )
endfunction()


################################################################################
# Shared library                                                               #
################################################################################

if (QSCHEMATIC_BUILD_SHARED)
    add_library(${TARGET_SHARED} SHARED)
    setup_target_common(${TARGET_SHARED})
endif()


################################################################################
# Static library                                                               #
################################################################################

if (QSCHEMATIC_BUILD_STATIC)
    add_library(${TARGET_STATIC} STATIC)
    setup_target_common(${TARGET_STATIC})

    target_compile_definitions(
        ${TARGET_STATIC}
            PUBLIC
            QSCHEMATIC_STATIC_DEFINE
    )
endif()


################################################################################
# Install                                                                      #
################################################################################

# Some basic stuff we'll need in this section
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
set(ConfigPackageLocation ${CMAKE_INSTALL_LIBDIR}/cmake/qschematic)

# Targets
if (QSCHEMATIC_BUILD_STATIC)
    list(APPEND INSTALL_TARGETS_LIST ${TARGET_STATIC})
endif()
if (QSCHEMATIC_BUILD_SHARED)
    list(APPEND INSTALL_TARGETS_LIST ${TARGET_SHARED})
endif()
install(
    TARGETS
        ${INSTALL_TARGETS_LIST}
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
