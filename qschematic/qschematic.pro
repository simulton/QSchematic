QT += core gui widgets xml
qtHaveModule(printsupport): QT += printsupport

TEMPLATE = lib
TARGET   = $$qtLibraryTarget(qschematic)
DESTDIR  = $$PWD

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++17 silent

# Include 3rdparty (GPDS) path
INCLUDEPATH += 3rdparty

# Include source files
include (qschematic.pri)

# Change this to build a dynamic/shared library
qschematicBuildStatic = true

!qschematicBuildStatic {
    CONFIG += sharedlib
}
qschematicBuildStatic {
    CONFIG += staticlib
}
