#pragma once

#include <memory>
#include <QMainWindow>
#include "../../../lib/settings.h"

class QUndoView;

namespace QSchematic {
    class Editor;
    class Scene;
    class View;
    class Wire;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

    bool save() const;
    bool load();
    void createActions();
    void demo();

private:
    void settingsChanged();
    std::unique_ptr<QSchematic::Wire> wireFactory() const;

    QSchematic::Editor* _schematicEditor;
    QSchematic::Scene* _scene;
    QSchematic::View* _view;
    QSchematic::Settings _settings;
    QUndoView* _undoView;
    QAction* _actionOpen;
    QAction* _actionSave;
    QAction* _actionUndo;
    QAction* _actionRedo;
    QAction* _actionModeNormal;
    QAction* _actionModeWire;
    QAction* _actionShowGrid;
    QAction* _actionRouteStraightAngles;
    QAction* _actionPreserveStraightAngles;
    QAction* _actionDebugMode;
};
