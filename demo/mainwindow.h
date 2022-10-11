#pragma once

#include <qschematic/settings.h>

#include <QMainWindow>

#include <memory>

#ifndef WINDOW_WIDTH
    #define WINDOW_WIDTH 2800
#endif
#ifndef WINDOW_HEIGHT
    #define WINDOW_HEIGHT 1500
#endif

class QUndoView;
class QGraphicsSceneContextMenuEvent;

namespace QSchematic
{
    class Scene;
    class View;
    class Wire;
}

namespace Library
{
    class Widget;
}

namespace Netlist
{
    class Viewer;
}

class MainWindow :
    public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MainWindow)

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

    bool save();
    bool load();
    bool load(const QString& filepath);
    void createActions();
    void demo();

private:
    void settingsChanged();
    void print();

    QSchematic::Scene* _scene;
    QSchematic::View* _view;
    QSchematic::Settings _settings;
    Library::Widget* _itemLibraryWidget;
    QUndoView* _undoView;
    ::Netlist::Viewer* _netlistViewer;
    QAction* _actionOpen;
    QAction* _actionSave;
    QAction* _actionPrint;
    QAction* _actionUndo;
    QAction* _actionRedo;
    QAction* _actionModeNormal;
    QAction* _actionModeWire;
    QAction* _actionShowGrid;
    QAction* _actionFitAll;
    QAction* _actionRouteStraightAngles;
    QAction* _actionGenerateNetlist;
    QAction* _actionDebugMode;
};
