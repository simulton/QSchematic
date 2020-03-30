[![Build Status](https://ci.simulton.com/buildStatus/icon?job=QSchematic&build=13)](https://ci.simulton.com/job/QSchematic/13/)

# Introduction
QSchematic is a library to draw diagrams & schematics with Qt. It uses Qt's [graphics view framework](http://doc.qt.io/qt-5/graphicsview.html).

The library provides base classes for objects such as nodes and wires and implements logic to move objects around while keeping the wires connected, generating netlists and so on. A typical application would include this library and subclass the `Item` class to implement custom items.

Feature overview:
  - Add, remove, move, resize nodes
  - Connect nodes with wires
  - Undo/redo
  - Drag'n'Drop
  - Template based netlist generation
  - Serialization to/from XML (powered by [GPDS](https://gpds.simulton.com))
  - Completely customizable by inheriting from the provided classes

Technical stuff:
  - Written in C++
  - Everything is contained within the `QSchematic` namespace
  - Tested with Qt5
  - No dependencies other than Qt5 and a C++17 compatible compiler
  - BSD 3-clause licensed

# State
This library is currently under heavy development.

# Licensing
The entire library is BSD 3-clause licensed.

# Credits
Special thank goes to Professor François Corthay (Switzerland) for initially privately funding this project.

# Screenshots
The library allows complete customization of every visual aspect. Therefore, screenshots are not really a telling thing as they are just showing an application specific implementation of the paint functions. But meh... I guess people still want to see some stuff so here we go:
![Screenshot 01](https://github.com/simulton/QSchematic/blob/master/docs/screenshots/screenshot_02.png?raw=true)

For more, check out the `docs/screenshots` folder.

You happy now?

# Building & integration
The QSchematic library depends on Qt5 and [GPDS](https://gpds.simulton.com) for (de)serialization.
Both QSchematic and GPDS are cmake projects. Building & integrating should therefore be straight forward. Follow these steps:

1. Download/clone, build & install [GPDS](https://gpds.simulton.com)
2. Download/clone QSchematic
3. Build the `qschematic-static` and/or `qschematic-shared` cmake targets to build a static and/or shared library version of QSchematic

The QSchematic static & shared library cmake targets are exported which allows for easy integration into a client application/library.
After successfully building & installing the QSchematic library targets, use `find_package()` to include the QSchematic targets and `target_link_libraries()` to add the corresponding target to your client application/library:
```
find_package(QSchematic REQUIRED)

add_executable(my_application "")
target_link_libraries(my_application qschematic::qschematic-static)
```
Check out the `/demo` for a practical hands-on example.


# Architecture
Did someone say UML?!

The class diagramm below shows the overall architecture of the system:
![System](https://github.com/simulton/QSchematic/blob/master/docs/uml/export/jpg/Model!QSchematic!System_1.jpg?raw=true)

All items that are part of the `Scene` inherit from the `Item` class. There are built-in specialized classes for nodes, wires and so on:
![Items](https://github.com/simulton/QSchematic/blob/master/docs/uml/export/jpg/Model!QSchematic!Items_0.jpg?raw=true)
