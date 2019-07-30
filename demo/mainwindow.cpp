#include <functional>
#include <memory>
#include <sstream>
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
#include <QFileDialog>
#include <QTextCodec>
#include <gpds/archiverxml.h>
#ifndef QT_NO_PRINTER
    #include <QPrinter>
    #include <QPrintDialog>
#endif
#include "../qschematic/scene.h"
#include "../qschematic/view.h"
#include "../qschematic/items/node.h"
#include "../qschematic/items/itemfactory.h"
#include "../qschematic/netlist.h"
#include "../qschematic/netlistgenerator.h"
#include "mainwindow.h"
#include "items/customitemfactory.h"
#include "items/operation.h"
#include "items/operationconnector.h"
#include "items/fancywire.h"
#include "itemslibrary/itemsslibrarywidget.h"

const QString FILE_FILTERS = "XML (*.xml)";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Setup the custom item factory
    auto func = std::bind(&CustomItemFactory::fromContainer, std::placeholders::_1);
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
        fileMenu->addSeparator();
        fileMenu->addAction(_actionPrint);

        // Menubar
        QMenuBar* menuBar = new QMenuBar;
        menuBar->addMenu(fileMenu);
        setMenuBar(menuBar);
    }

    // Toolbar
    QToolBar* editorToolbar = new QToolBar;
    editorToolbar->addAction(_actionUndo);
    editorToolbar->addAction(_actionRedo);
    editorToolbar->addSeparator();
    editorToolbar->addAction(_actionModeNormal);
    editorToolbar->addAction(_actionModeWire);
    editorToolbar->addSeparator();
    editorToolbar->addAction(_actionRouteStraightAngles);
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

bool MainWindow::save()
{
    // Prompt for a path
    QString path = QFileDialog::getSaveFileName(this, "Save to file", QDir::homePath(), FILE_FILTERS);
    if (path.isEmpty()) {
        return false;
    }

    // Open the file
    QFile file(path);
    file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);
    if (!file.isOpen()) {
        return false;
    }

    // Archiver
    Gpds::ArchiverXml ar;
    std::stringstream stream;
    ar.save(stream, *_scene, "qschematic");

    // Write file
    file.write( QByteArray::fromStdString( stream.str() ) );
    file.flush();
    file.close();

    return true;
}

bool MainWindow::load()
{
    // Prompt for a path
    QString path = QFileDialog::getOpenFileName(this, "Load from file", QDir::homePath(), FILE_FILTERS);
    if (path.isEmpty()) {
        return false;
    }

    // Get rid of everything existing
    _scene->clear();

    // Open the file
    QFile file(path);
    file.open(QFile::ReadOnly);
    if (!file.isOpen()) {
        return false;
    }

    // Archiver
    Gpds::ArchiverXml ar;
    std::stringstream stream;
    stream << file.readAll().data();
    ar.load(stream, *_scene, "qschematic");

    // Clean up
    file.close();

    return true;
}

void MainWindow::createActions()
{
    // Open
    _actionOpen = new QAction;
    _actionOpen->setText("Open");
    _actionOpen->setIcon( QIcon( ":/folder_open.svg" ) );
    _actionOpen->setToolTip("Open a file");
    connect(_actionOpen, &QAction::triggered, [this]{
        load();
    });

    // Save
    _actionSave = new QAction;
    _actionSave->setText("Save");
    _actionSave->setToolTip("Save to a file");
    _actionSave->setIcon( QIcon( ":/save.svg" ) );
    connect(_actionSave, &QAction::triggered, [this]{
        save();
    });

    // Print
    _actionPrint = new QAction;
    _actionPrint->setText("Print");
    _actionPrint->setShortcut(QKeySequence::Print);
    _actionPrint->setIcon( QIcon( ":/print.svg" ) );
    connect(_actionPrint, &QAction::triggered, [this]{
        print();
    });

    // Undo
    _actionUndo = _scene->undoStack()->createUndoAction(this, QStringLiteral("Undo"));
    _actionUndo->setIcon( QIcon( ":/undo.svg") );
    _actionUndo->setText("Undo");
    _actionUndo->setShortcut(QKeySequence::Undo);

    // Redo
    _actionRedo = _scene->undoStack()->createRedoAction(this, QStringLiteral("Redo"));
    _actionRedo->setIcon( QIcon( ":/redo.svg") );
    _actionRedo->setText("Redo");
    _actionRedo->setShortcut(QKeySequence::Redo);

    // Mode: Normal
    _actionModeNormal = new QAction("Normal Mode", this);
    _actionModeNormal->setIcon( QIcon( ":/mode_normal.svg") );
    _actionModeNormal->setToolTip("Change to the normal mode (allows to move components).");
    _actionModeNormal->setCheckable(true);
    _actionModeNormal->setChecked(true);
    connect(_actionModeNormal, &QAction::triggered, [this]{ _scene->setMode(QSchematic::Scene::NormalMode); });

    // Mode: Wire
    _actionModeWire = new QAction("Wire Mode", this);
    _actionModeWire->setIcon( QIcon( ":/mode_wire.svg") );
    _actionModeWire->setToolTip("Change to the wire mode (allows to draw wires).");
    _actionModeWire->setCheckable(true);
    connect(_actionModeWire, &QAction::triggered, [this]{ _scene->setMode(QSchematic::Scene::WireMode); });

    // Mode action group
    QActionGroup* actionGroupMode = new QActionGroup(this);
    actionGroupMode->addAction(_actionModeNormal);
    actionGroupMode->addAction(_actionModeWire);

    // Show grid
    _actionShowGrid = new QAction("Toggle Grid");
    _actionShowGrid->setIcon( QIcon( ":/grid.svg") );
    _actionShowGrid->setCheckable(true);
    _actionShowGrid->setChecked(_settings.showGrid);
    _actionShowGrid->setToolTip("Toggle grid visibility");
    connect(_actionShowGrid, &QAction::toggled, [this](bool checked){
        _settings.showGrid = checked;
        settingsChanged();
    });

    // Route straight angles
    _actionRouteStraightAngles = new QAction("Wire angles");
    _actionRouteStraightAngles->setIcon( QIcon( ":/wire_rightangle.svg") );
    _actionRouteStraightAngles->setCheckable(true);
    _actionRouteStraightAngles->setChecked(_settings.routeStraightAngles);
    connect(_actionRouteStraightAngles, &QAction::toggled, [this](bool checked){
        _settings.routeStraightAngles = checked;
        _settings.preserveStraightAngles = checked;
        settingsChanged();
    });

    // Generate netlist
    _actionGenerateNetlist = new QAction("Generate netlist");
    _actionGenerateNetlist->setIcon( QIcon( ":/netlist.svg" ) );
    connect(_actionGenerateNetlist, &QAction::triggered, [this]{
        QSchematic::Netlist<> netlist;
        QSchematic::NetlistGenerator::generate(netlist, *_scene);
        QJsonDocument document(netlist.toJson());

        QInputDialog::getMultiLineText(this, "Netlist", "Generated netlist", document.toJson(QJsonDocument::Indented));
    });

    // Debug mode
    _actionDebugMode = new QAction("Debug");
    _actionDebugMode->setCheckable(true);
    _actionDebugMode->setIcon( QIcon( ":/bug.svg") );
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

void MainWindow::print()
{
    Q_ASSERT(_scene);
#ifndef QT_NO_PRINTER

    QPrinter printer(QPrinter::HighResolution);
    if (QPrintDialog(&printer).exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        painter.setRenderHint(QPainter::Antialiasing);
        _scene->render(&painter);
    }

#endif // QT_NO_PRINTER
}

void MainWindow::demo()
{
    _scene->clear();
    _scene->setSceneRect(-500, -500, 3000, 3000);

    auto o1 = std::make_shared<Operation>();
    o1->addConnector(std::make_shared<OperationConnector>(QPoint(0, 2), QStringLiteral("in")));
    o1->addConnector(std::make_shared<OperationConnector>(QPoint(8, 2), QStringLiteral("out")));
    o1->setGridPos(0, 0);
    o1->setSize(160, 80);
    o1->setConnectorsMovable(true);
    o1->setText(QStringLiteral("Operation 1"));
    o1->label()->setPos(0, 110);
    _scene->addItem(o1);

    auto o2 = std::make_shared<Operation>();
    o2->addConnector(std::make_shared<OperationConnector>(QPoint(0, 2), QStringLiteral("in")));
    o2->addConnector(std::make_shared<OperationConnector>(QPoint(8, 2), QStringLiteral("out")));
    o2->setGridPos(-14, 9);
    o2->setSize(160, 80);
    o2->setConnectorsMovable(true);
    o2->setText(QStringLiteral("Operation 2"));
    o2->label()->setPos(0, 110);
    _scene->addItem(o2);

    auto o3 = std::make_shared<Operation>();
    o3->setSize(160, 120);
    o3->addConnector(std::make_shared<OperationConnector>(QPoint(0, 2), QStringLiteral("in 1")));
    o3->addConnector(std::make_shared<OperationConnector>(QPoint(0, 4), QStringLiteral("in 2")));
    o3->addConnector(std::make_shared<OperationConnector>(QPoint(8, 3), QStringLiteral("out")));
    o3->setGridPos(18, -8);
    o3->setConnectorsMovable(true);
    o3->setText(QStringLiteral("Operation 3"));
    o3->label()->setPos(0, 150);
    _scene->addItem(o3);

    auto o4 = std::make_shared<Operation>();
    o4->setSize(160, 120);
    o4->addConnector(std::make_shared<OperationConnector>(QPoint(0, 2), QStringLiteral("in 1")));
    o4->addConnector(std::make_shared<OperationConnector>(QPoint(0, 4), QStringLiteral("in 2")));
    o4->addConnector(std::make_shared<OperationConnector>(QPoint(8, 3), QStringLiteral("out")));
    o4->setGridPos(18, 10);
    o4->setConnectorsMovable(true);
    o4->setText(QStringLiteral("Operation 4"));
    o4->label()->setPos(0, 150);
    _scene->addItem(o4);

    _scene->undoStack()->clear();
}
