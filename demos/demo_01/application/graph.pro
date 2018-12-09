#-------------------------------------------------
#
# Project created by QtCreator 2018-10-27T01:15:43
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = graph
TEMPLATE = app

include (../../../lib/qschematics.pri)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14 silent

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    resources.cpp \
    commands/commandnodeaddconnector.cpp \
    items/customitemfactory.cpp \
    items/fancywire.cpp \
    items/operation.cpp \
    items/operationconnector.cpp \
    itemslibrary/itemslibrarymodel.cpp \
    itemslibrary/itemslibraryview.cpp \
    itemslibrary/itemsslibrarywidget.cpp \
    commands/commandlabelrename.cpp

HEADERS += \
        mainwindow.h \
    resources.h \
    commands/commands.h \
    commands/commandnodeaddconnector.h \\
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
    commands/commandlabelrename.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources/resources.qrc
