# Introduction
QSchematic is a library to draw diagrams & schematics with Qt. It uses Qt's [graphics view framework](http://doc.qt.io/qt-5/graphicsview.html).

The library provides base classes for objects such as nodes and wires and implements logic to move objects around while keeping the wires connected, generating netlists and so on. A typical application would include this library and subclass the `Item` class to implement custom items.

The library is written in C++.

# State
This library is currently under heavy development.

# Compatibility
This library is known to work with Qt 5.

# Dependencies
  - Qt 5
  - C++11 compatible compiler

# Licensing
The entire library is BSD 3-clause licensed.

# Credits
Special thank goes to Professor François Corthay (Switzerland) for privately funding this project.

# Architecture
Did someone say UML?!

The class diagramm below shows the overall architecture of the system:
![System](https://github.com/simulton/QSchematic/blob/master/docs/uml/export/jpg/Model!SchematicEditor!System_2.jpg?raw=true)

All items that are part of the `Scene` inherit from the `Item` class. There are built-in specialized classes for nodes, wires and so on:
![Items](https://github.com/simulton/QSchematic/blob/master/docs/uml/export/jpg/Model!SchematicEditor!Items_1.jpg?raw=true)
