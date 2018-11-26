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
