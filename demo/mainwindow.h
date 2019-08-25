#pragma once

#include <memory>
#include <QMainWindow>
#include "../qschematic/settings.h"

class QUndoView;
class QGraphicsSceneContextMenuEvent;

namespace QSchematic {
    class Editor;
    class Scene;
    class View;
    class Wire;
}

class ItemsLibraryWidget;
class NetlistViewer;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

    bool save();
    bool load();
    void createActions();
    void demo();

private:
    void settingsChanged();
    void print();

    QSchematic::Editor* _schematicEditor;
    QSchematic::Scene* _scene;
    QSchematic::View* _view;
    QSchematic::Settings _settings;
    ItemsLibraryWidget* _itemLibraryWidget;
    QUndoView* _undoView;
    NetlistViewer* _netlistViewer;
    QAction* _actionOpen;
    QAction* _actionSave;
    QAction* _actionPrint;
    QAction* _actionUndo;
    QAction* _actionRedo;
    QAction* _actionModeNormal;
    QAction* _actionModeWire;
    QAction* _actionShowGrid;
    QAction* _actionRouteStraightAngles;
    QAction* _actionGenerateNetlist;
    QAction* _actionDebugMode;
};
