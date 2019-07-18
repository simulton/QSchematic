QT += core gui widgets xml
qtHaveModule(printsupport): QT += printsupport

TARGET = qschematic_demo
TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += c++17 silent

# Include GPDS (as source)
include(../qschematic/qschematic.pri)
INCLUDEPATH += $$PWD/../qschematic

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    resources.cpp \
    items/customitemfactory.cpp \
    items/fancywire.cpp \
    items/operation.cpp \
    items/operationconnector.cpp \
    itemslibrary/itemslibrarymodel.cpp \
    itemslibrary/itemslibraryview.cpp \
    itemslibrary/itemsslibrarywidget.cpp \
    items/operationdemo1.cpp \
    commands/commandnodeaddconnector.cpp \
    items/flowstart.cpp \
    items/flowend.cpp

HEADERS += \
    mainwindow.h \
    resources.h \
    commands/commands.h \
    commands/commandnodeaddconnector.h \
    items/customitemfactory.h \
    items/fancywire.h \
    items/itemtypes.h \
    items/operation.h \
    items/operationconnector.h \
    itemslibrary/itemslibrarymodel.h \
    itemslibrary/itemslibrarymodelitem.h \
    itemslibrary/itemslibraryview.h \
    itemslibrary/itemsslibrarywidget.h \
    itemslibrary/iteminfo.h \
    items/operationdemo1.h \
    items/flowstart.h \
    items/flowend.h

RESOURCES += \
    resources/resources.qrc
