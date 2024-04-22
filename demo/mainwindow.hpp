#pragma once

#include <qschematic/settings.hpp>

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
    class Widget;
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
    void generateNetlist();

    QSchematic::Scene* _scene = nullptr;
    QSchematic::View* _view = nullptr;
    QSchematic::Settings _settings;
    Library::Widget* _itemLibraryWidget = nullptr;
    QUndoView* _undoView = nullptr;
    ::Netlist::Widget* _netlistViewerWidget = nullptr;
    QAction* _actionOpen = nullptr;
    QAction* _actionSave = nullptr;
    QAction* _actionPrint = nullptr;
    QAction* _actionUndo = nullptr;
    QAction* _actionRedo = nullptr;
    QAction* _actionModeNormal = nullptr;
    QAction* _actionModeWire = nullptr;
    QAction* _actionShowGrid = nullptr;
    QAction* _actionFitAll = nullptr;
    QAction* _actionRouteStraightAngles = nullptr;
    QAction* _actionGenerateNetlist = nullptr;
    QAction* _actionClear = nullptr;
    QAction* _actionDebugMode = nullptr;
};
