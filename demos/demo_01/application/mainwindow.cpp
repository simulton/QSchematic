#include <QToolBar>
#include <QSlider>
#include <QLabel>
#include <QAction>
#include "../../../lib/scene.h"
#include "../../../lib/view.h"
#include "../../../lib/settings.h"
#include "../../../lib/items/node.h"
#include "mainwindow.h"
#include "items/operation.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    createActions();

    // Settings
    _settings.debug = false;
    _settings.routeStraightAngles = false;

    // Scene
    _scene = new QSchematic::Scene;
    _scene->setSettings(_settings);

    // View
    _view = new QSchematic::View;
    _view->setSettings(_settings);
    _view->setScene(_scene);

    // Grid size slider
    QSlider* gridSize = new QSlider(Qt::Horizontal);
    gridSize->setRange(1, 100);
    gridSize->setValue(20);
    connect(gridSize, &QSlider::valueChanged, [this](int value){
        _settings.gridSize = value;
        _scene->setSettings(_settings);
    });

    // Toolbar
    QToolBar* toolBar = new QToolBar;
    toolBar->addWidget(new QLabel("Grid size:"));
    toolBar->addWidget(gridSize);
    toolBar->addSeparator();
    toolBar->addAction(_actionModeNormal);
    toolBar->addAction(_actionModeWire);
    toolBar->addAction(_routeStraightAngles);
    addToolBar(toolBar);

    // Misc toolbar
    QToolBar* miscToolbar = new QToolBar;
    miscToolbar->addAction(_actionDebugMode);
    addToolBar(miscToolbar);

    // Central widget
    setCentralWidget(_view);

    // Misc
    setWindowTitle("Schematic Editor");
    resize(1920, 1080);

    demo();
}

void MainWindow::createActions()
{
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

    // Route straight angles
    _routeStraightAngles = new QAction("Wire angles");
    _routeStraightAngles->setCheckable(true);
    _routeStraightAngles->setChecked(_settings.routeStraightAngles);
    connect(_routeStraightAngles, &QAction::toggled, [this](bool checked){
        _settings.routeStraightAngles = checked;
        settingsChanged();
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
    _scene->setSceneRect(-300, -300, 5000, 5000);

    QSchematic::Node* n1 = new QSchematic::Node();
    n1->setGridPoint(-5, -2);
    n1->addConnector(new QSchematic::Connector(QPoint(0, 3), QStringLiteral("Foo")));
    n1->addConnector(new QSchematic::Connector(QPoint(0, 5), QStringLiteral("Bar")));
    n1->setConnectorsMovable(true);
    _scene->addItem(n1);

    QSchematic::Node* n2 = new QSchematic::Node();
    n2->setGridPoint(6, 11);
    n2->addConnector(new QSchematic::Connector(QPoint(0, 2), QStringLiteral("Connector")));
    _scene->addItem(n2);

    Operation* o = new Operation;
    o->setGridPoint(4, 15);
    o->setConnectorsMovable(true);
    _scene->addItem(o);
}
