#include <functional>
#include <memory>
#include <QToolBar>
#include <QSlider>
#include <QLabel>
#include <QAction>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QMenuBar>
#include <QMenu>
#include <QUndoView>
#include <QDockWidget>
#include <QGraphicsSceneContextMenuEvent>
#include <QInputDialog>
#include "../../../lib/scene.h"
#include "../../../lib/view.h"
#include "../../../lib/settings.h"
#include "../../../lib/items/node.h"
#include "../../../lib/items/label.h"
#include "../../../lib/items/itemfactory.h"
#include "../../../lib/netlist.h"
#include "../../../lib/netlistgenerator.h"
#include "mainwindow.h"
#include "resources.h"
#include "items/customitemfactory.h"
#include "items/operation.h"
#include "items/operationconnector.h"
#include "items/fancywire.h"
#include "itemslibrary/itemsslibrarywidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Setup the custom item factory
    auto func = std::bind(&CustomItemFactory::fromJson, std::placeholders::_1);
    QSchematic::ItemFactory::instance().setCustomItemsFactory(func);

    // Settings
    _settings.debug = false;
    _settings.showGrid = false;
    _settings.routeStraightAngles = true;

    // Scene (object needed in createActions())
    _scene = new QSchematic::Scene;

    // Create actions
    createActions();

    // Scene
    _scene->setSettings(_settings);
    _scene->setWireFactory([]{ return std::make_unique<FancyWire>(); });
    connect(_scene, &QSchematic::Scene::modeChanged, [this](QSchematic::Scene::Mode mode){
        switch (mode) {
        case QSchematic::Scene::NormalMode:
            _actionModeNormal->setChecked(true);
            break;

        case QSchematic::Scene::WireMode:
            _actionModeWire->setChecked(true);
            break;
        }
    });

    // View
    _view = new QSchematic::View;
    _view->setSettings(_settings);
    _view->setScene(_scene);

    // Item library
    _itemLibraryWidget = new ItemsLibraryWidget(this);
    connect(_view, &QSchematic::View::zoomChanged, _itemLibraryWidget, &ItemsLibraryWidget::setPixmapScale);
    QDockWidget* itemLibraryDock = new QDockWidget;
    itemLibraryDock->setWindowTitle("Items");
    itemLibraryDock->setWidget(_itemLibraryWidget);
    addDockWidget(Qt::LeftDockWidgetArea, itemLibraryDock);

    // Undo view
    _undoView = new QUndoView(_scene->undoStack());
    QDockWidget* undoDockWiget = new QDockWidget;
    undoDockWiget->setWindowTitle("Command histoy");
    undoDockWiget->setWidget(_undoView);
    addDockWidget(Qt::LeftDockWidgetArea, undoDockWiget);

    // Menus
    {
        // File menu
        QMenu* fileMenu = new QMenu(QStringLiteral("&File"));
        fileMenu->addAction(_actionOpen);
        fileMenu->addAction(_actionSave);

        // Menubar
        QMenuBar* menuBar = new QMenuBar;
        menuBar->addMenu(fileMenu);
        setMenuBar(menuBar);
    }

    // Toolbar
    QToolBar* editorToolbar = new QToolBar;
    editorToolbar->addAction(_actionUndo);
    editorToolbar->addAction(_actionRedo);
    editorToolbar->addAction(_actionModeNormal);
    editorToolbar->addAction(_actionModeWire);
    editorToolbar->addSeparator();
    editorToolbar->addAction(_actionRouteStraightAngles);
    editorToolbar->addAction(_actionPreserveStraightAngles);
    editorToolbar->addSeparator();
    editorToolbar->addAction(_actionGenerateNetlist);
    addToolBar(editorToolbar);

    // View toolbar
    QToolBar* viewToolbar = new QToolBar;
    viewToolbar->addAction(_actionShowGrid);
    addToolBar(viewToolbar);

    // Debug toolbar
    QToolBar* debugToolbar = new QToolBar;
    debugToolbar->addAction(_actionDebugMode);
    addToolBar(debugToolbar);

    // Central widget
    setCentralWidget(_view);

    // Misc
    setWindowTitle("Schematic Editor");
    resize(2800, 1500);
    _view->setZoomValue(1.5);

    demo();
}

bool MainWindow::save() const
{
    QJsonDocument document(_scene->toJson());

    QFile file(QDir::homePath() + "/Documents/junk/qschematic.json");
    file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);
    if (!file.isOpen()) {
        return false;
    }
    file.write(document.toJson());

    return true;
}

bool MainWindow::load()
{
    _scene->clear();

    QFile file(QDir::homePath() + "/Documents/junk/qschematic.json");
    file.open(QFile::ReadOnly);
    if (!file.isOpen()) {
        return false;
    }

    QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    _scene->fromJson(document.object());

    return true;
}

void MainWindow::createActions()
{
    // Open
    _actionOpen = new QAction;
    _actionOpen->setText("Open");
    _actionOpen->setToolTip("Open a file");
    connect(_actionOpen, &QAction::triggered, [this]{
        load();
    });

    // Save
    _actionSave = new QAction;
    _actionSave->setText("Save");
    _actionSave->setToolTip("Save to a file");
    connect(_actionSave, &QAction::triggered, [this]{
        save();
    });

    // Undo
    _actionUndo = _scene->undoStack()->createUndoAction(this, QStringLiteral("Undo"));
    _actionUndo->setText("Undo");
    _actionUndo->setShortcut(QKeySequence::Undo);

    // Redo
    _actionRedo = _scene->undoStack()->createRedoAction(this, QStringLiteral("Redo"));
    _actionRedo->setText("Redo");
    _actionRedo->setShortcut(QKeySequence::Redo);

    // Mode: Normal
    _actionModeNormal = new QAction("Normal Mode", this);
    _actionModeNormal->setToolTip("Change to the normal mode (allows to move components).");
    _actionModeNormal->setCheckable(true);
    _actionModeNormal->setChecked(true);
    connect(_actionModeNormal, &QAction::triggered, [this]{ _scene->setMode(QSchematic::Scene::NormalMode); });

    // Mode: Wire
    _actionModeWire = new QAction("Wire Mode", this);
    _actionModeWire->setToolTip("Change to the wire mode (allows to draw wires).");
    _actionModeWire->setCheckable(true);
    connect(_actionModeWire, &QAction::triggered, [this]{ _scene->setMode(QSchematic::Scene::WireMode); });

    // Mode action group
    QActionGroup* actionGroupMode = new QActionGroup(this);
    actionGroupMode->addAction(_actionModeNormal);
    actionGroupMode->addAction(_actionModeWire);

    // Show grid
    _actionShowGrid = new QAction;
    _actionShowGrid->setCheckable(true);
    _actionShowGrid->setChecked(_settings.showGrid);
    _actionShowGrid->setToolTip("Toggle grid visibility");
    _actionShowGrid->setIcon(Resources::icon(Resources::ToggleGridIcon));
    connect(_actionShowGrid, &QAction::toggled, [this](bool checked){
        _settings.showGrid = checked;
        settingsChanged();
    });

    // Route straight angles
    _actionRouteStraightAngles = new QAction("Wire angles");
    _actionRouteStraightAngles->setCheckable(true);
    _actionRouteStraightAngles->setChecked(_settings.routeStraightAngles);
    connect(_actionRouteStraightAngles, &QAction::toggled, [this](bool checked){
        _settings.routeStraightAngles = checked;
        settingsChanged();
    });

    // Preserve straight angles
    _actionPreserveStraightAngles = new QAction("Preserve angles");
    _actionPreserveStraightAngles->setCheckable(true);
    _actionPreserveStraightAngles->setChecked(_settings.preserveStraightAngles);
    connect(_actionPreserveStraightAngles, &QAction::toggled, [this](bool checked){
        _settings.preserveStraightAngles = checked;
        settingsChanged();
    });

    _actionGenerateNetlist = new QAction("Generate netlist");
    connect(_actionGenerateNetlist, &QAction::triggered, [this]{
        auto netlist = QSchematic::NetlistGenerator::generate(*_scene);
        QJsonDocument document(netlist.toJson());

        QInputDialog::getMultiLineText(this, "Netlist", "Generated netlist", document.toJson(QJsonDocument::Indented));
    });

    // Debug mode
    _actionDebugMode = new QAction("Debug");
    _actionDebugMode->setCheckable(true);
    _actionDebugMode->setChecked(_settings.debug);
    connect(_actionDebugMode, &QAction::toggled, [this](bool checked){
        _settings.debug = checked;
        settingsChanged();
    });
}

void MainWindow::settingsChanged()
{
    _view->setSettings(_settings);
    _scene->setSettings(_settings);
}

void MainWindow::demo()
{
    _scene->clear();
    _scene->setSceneRect(-500, -500, 3000, 3000);

    Operation* o1 = new Operation;
    o1->addConnector(std::make_shared<OperationConnector>(QPoint(0, 2), QStringLiteral("in")));
    o1->addConnector(std::make_shared<OperationConnector>(QPoint(8, 2), QStringLiteral("out")));
    o1->setGridPos(-7, -6);
    o1->setConnectorsMovable(true);
    o1->label()->setText(QStringLiteral("Operation 1"));
    _scene->addItem(o1);

    Operation* o2 = new Operation;
    o2->addConnector(std::make_shared<OperationConnector>(QPoint(0, 2), QStringLiteral("in")));
    o2->addConnector(std::make_shared<OperationConnector>(QPoint(8, 2), QStringLiteral("out")));
    o2->setGridPos(-4, 6);
    o2->setConnectorsMovable(true);
    o1->label()->setText(QStringLiteral("Operation 2"));
    _scene->addItem(o2);

    Operation* o3 = new Operation;
    o3->setSize(8, 6);
    o3->addConnector(std::make_shared<OperationConnector>(QPoint(0, 2), QStringLiteral("in 1")));
    o3->addConnector(std::make_shared<OperationConnector>(QPoint(0, 4), QStringLiteral("in 2")));
    o3->addConnector(std::make_shared<OperationConnector>(QPoint(8, 3), QStringLiteral("out")));
    o3->setGridPos(12, -2);
    o3->setConnectorsMovable(true);
    o3->label()->setText(QStringLiteral("Operation 3"));
    _scene->addItem(o3);

    _scene->undoStack()->clear();
}
