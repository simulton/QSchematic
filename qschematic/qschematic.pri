# Set the path to GPDS to our built-in version (git
# submodule)
#
# This allows the user to use a different version of
# GPDS with QSchematic. This also comes in handy when
# using the QSchematic library in an environment that
# already uses/provides GPDS itself
isEmpty(GPDS_PATH) {
    GPDS_PATH = 3rdparty/gpds
}

include($${GPDS_PATH}/src/gpds.pri)

INCLUDEPATH += $$_PRO_FILE_PWD_

HEADERS += \
    $$PWD/items/connector.h \
    $$PWD/items/item.h \
    $$PWD/items/label.h \
    $$PWD/items/line.h \
    $$PWD/items/node.h \
    $$PWD/items/wire.h \
    $$PWD/items/wirenet.h \
    $$PWD/items/wirepoint.h \
    $$PWD/scene.h \
    $$PWD/settings.h \
    $$PWD/types.h \
    $$PWD/view.h \
    $$PWD/utils.h \
    $$PWD/items/itemfactory.h \
    $$PWD/items/wireroundedcorners.h \
    $$PWD/items/itemmimedata.h \
    $$PWD/netlist.h \
    $$PWD/netlistgenerator.h \
    $$PWD/commands/commanditemadd.h \
    $$PWD/commands/commanditemmove.h \
    $$PWD/commands/commanditemremove.h \
    $$PWD/commands/commanditemvisibility.h \
    $$PWD/commands/commandlabelrename.h \
    $$PWD/commands/commandnoderesize.h \
    $$PWD/commands/commands.h \
    $$PWD/items/splinewire.h \
    $$PWD/commands/commandnoderotate.h

SOURCES += \
    $$PWD/items/connector.cpp \
    $$PWD/items/item.cpp \
    $$PWD/items/label.cpp \
    $$PWD/items/line.cpp \
    $$PWD/items/node.cpp \
    $$PWD/items/wire.cpp \
    $$PWD/items/wirenet.cpp \
    $$PWD/items/wirepoint.cpp \
    $$PWD/scene.cpp \
    $$PWD/settings.cpp \
    $$PWD/view.cpp \
    $$PWD/utils.cpp \
    $$PWD/items/itemfactory.cpp \
    $$PWD/items/wireroundedcorners.cpp \
    $$PWD/items/itemmimedata.cpp \
    $$PWD/commands/commanditemadd.cpp \
    $$PWD/commands/commanditemmove.cpp \
    $$PWD/commands/commanditemremove.cpp \
    $$PWD/commands/commanditemvisibility.cpp \
    $$PWD/commands/commandlabelrename.cpp \
    $$PWD/commands/commandnoderesize.cpp \
    $$PWD/items/splinewire.cpp \
    $$PWD/commands/commandnoderotate.cpp
