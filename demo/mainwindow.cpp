#include "mainwindow.hpp"
#include "items/customitemfactory.hpp"
#include "items/operation.hpp"
#include "items/operationconnector.hpp"
#include "items/fancywire.hpp"
#include "items/widgets/dial.hpp"
#include "library/widget.hpp"
#include "netlist/widget.hpp"

#include <gpds/archiver_xml.hpp>
#include <qschematic/scene.hpp>
#include <qschematic/view.hpp>
#include <qschematic/commands/item_add.hpp>
#include <qschematic/items/node.hpp>
#include <qschematic/items/itemfactory.hpp>
#include <qschematic/items/widget.hpp>
#include <qschematic/netlist.hpp>
#include <qschematic/netlistgenerator.hpp>

#include <QToolBar>
#include <QAction>
#include <QActionGroup>
#include <QFile>
#include <QDir>
#include <QMenuBar>
#include <QMenu>
#include <QUndoView>
#include <QDockWidget>
#include <QGraphicsSceneContextMenuEvent>
#include <QInputDialog>
#include <QFileDialog>
#include <QtDebug>
#ifndef QT_NO_PRINTER
    #include <QPrinter>
    #include <QPrintDialog>
#endif

#include <functional>
#include <memory>
#include <sstream>

const QString FILE_FILTERS = "XML (*.xml)";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Setup the custom item factory
    auto func = std::bind(&CustomItemFactory::from_container, std::placeholders::_1);
    QSchematic::Items::Factory::instance().setCustomItemsFactory(func);

    // Settings
    _settings.debug = false;
    _settings.showGrid = false;
    _settings.routeStraightAngles = true;

    // Scene (object needed in createActions())
    _scene = new QSchematic::Scene(this);

    // Create actions
    createActions();

    // Scene
    _scene->setSettings(_settings);
    _scene->setWireFactory([]{ return std::make_shared<FancyWire>(); });
    connect(_scene, &QSchematic::Scene::modeChanged, [this](int mode){
        switch (mode) {
        case QSchematic::Scene::NormalMode:
            _actionModeNormal->setChecked(true);
            break;

        case QSchematic::Scene::WireMode:
            _actionModeWire->setChecked(true);
            break;

        default:
            break;
        }
    });
    connect(_scene, &QSchematic::Scene::netlistChanged, [this](){
        qDebug() << "Netlist changed";

        generateNetlist();
    });

    // View
    _view = new QSchematic::View(this);
    _view->setSettings(_settings);
    _view->setScene(_scene);

    // Item library
    _itemLibraryWidget = new Library::Widget(this);
    connect(_view, &QSchematic::View::zoomChanged, _itemLibraryWidget, &Library::Widget::setPixmapScale);
    QDockWidget* itemLibraryDock = new QDockWidget;
    itemLibraryDock->setWindowTitle("Items");
    itemLibraryDock->setWidget(_itemLibraryWidget);
    addDockWidget(Qt::LeftDockWidgetArea, itemLibraryDock);

    // Undo view
    _undoView = new QUndoView(_scene->undoStack(), this);
    QDockWidget* undoDockWiget = new QDockWidget;
    undoDockWiget->setWindowTitle("Command History");
    undoDockWiget->setWidget(_undoView);
    addDockWidget(Qt::LeftDockWidgetArea, undoDockWiget);

    // Netlist viewer widget
    _netlistViewerWidget = new ::Netlist::Widget(this);
    QDockWidget* netlistviewerDockWidget = new QDockWidget;
    netlistviewerDockWidget->setWindowTitle(QStringLiteral("Netlist Viewer"));
    netlistviewerDockWidget->setWidget(_netlistViewerWidget);
    addDockWidget(Qt::LeftDockWidgetArea, netlistviewerDockWidget);

    // Menus
    {
        // File menu
        QMenu* fileMenu = new QMenu(QStringLiteral("&File"), this);
        fileMenu->addAction(_actionOpen);
        fileMenu->addAction(_actionSave);
        fileMenu->addSeparator();
        fileMenu->addAction(_actionPrint);

        // Menubar
        QMenuBar* menuBar = new QMenuBar(this);
        menuBar->addMenu(fileMenu);
        setMenuBar(menuBar);
    }

    // Toolbar
    QToolBar* editorToolbar = new QToolBar(this);
    editorToolbar->addAction(_actionUndo);
    editorToolbar->addAction(_actionRedo);
    editorToolbar->addSeparator();
    editorToolbar->addAction(_actionModeNormal);
    editorToolbar->addAction(_actionModeWire);
    editorToolbar->addSeparator();
    editorToolbar->addAction(_actionRouteStraightAngles);
    editorToolbar->addSeparator();
    editorToolbar->addAction(_actionGenerateNetlist);
    editorToolbar->addAction(_actionClear);
    addToolBar(editorToolbar);

    // View toolbar
    QToolBar* viewToolbar = new QToolBar(this);
    viewToolbar->addAction(_actionShowGrid);
    viewToolbar->addAction(_actionFitAll);
    addToolBar(viewToolbar);

    // Debug toolbar
    QToolBar* debugToolbar = new QToolBar(this);
    debugToolbar->addAction(_actionDebugMode);
    addToolBar(debugToolbar);

    // Central widget
    setCentralWidget(_view);

    // Misc
    setWindowTitle("QSchematic Editor");
    resize( WINDOW_WIDTH, WINDOW_HEIGHT );
#ifdef WINDOW_MAXIMIZE
    setWindowState(Qt::WindowMaximized);
#endif

    demo();
}

bool MainWindow::save()
{
    // Prompt for a path
    const std::filesystem::path path = QFileDialog::getSaveFileName(this, "Save to file", QDir::homePath(), FILE_FILTERS).toStdString();
    if (path.empty())
        return false;

    // Serialize with GPDS
    const auto& [success, message] = gpds::to_file<gpds::archiver_xml>(path, *_scene);
    if (!success) {
        qWarning() << "could not save to file: " << QString::fromStdString(message);
        return false;
    }

    return true;
}

bool MainWindow::load()
{
    // Prompt for a path
    QString path = QFileDialog::getOpenFileName(this, "Load from file", QDir::homePath(), FILE_FILTERS);
    if (path.isEmpty())
        return false;

    return load(path);
}

bool MainWindow::load(const QString& filepath)
{
    // Note: We don't use gpds::from_file() in here because we're potentially loading demo files from Qt's resource
    //       system. In such a case, those files can't be accessed via std::filesystem::path.

    // Get rid of everything existing
    _scene->clear();

    // Open the file
    QFile file(filepath);
    file.open(QFile::ReadOnly);
    if (!file.isOpen())
        return false;
    std::stringstream stream;
    stream << file.readAll().data();

    // Archiver
    const auto& [success, message] = gpds::from_stream<gpds::archiver_xml>(stream, *_scene, QSchematic::Scene::gpds_name);
    if (!success) {
        qDebug() << "MainWindow::load(): Could not load scene: " << QString::fromStdString(message);
        return false;
    }

    // Clean up
    file.close();

    return true;
}

void MainWindow::createActions()
{
    // Open
    _actionOpen = new QAction(this);
    _actionOpen->setText("Open");
    _actionOpen->setIcon( QIcon( ":/folder_open.svg" ) );
    _actionOpen->setToolTip("Open a file");
    _actionOpen->setShortcut(QKeySequence::Open);
    connect(_actionOpen, &QAction::triggered, [this]{
        load();
    });

    // Save
    _actionSave = new QAction(this);
    _actionSave->setText("Save");
    _actionSave->setToolTip("Save to a file");
    _actionSave->setIcon( QIcon( ":/save.svg" ) );
    _actionSave->setShortcut(QKeySequence::Save);
    connect(_actionSave, &QAction::triggered, [this]{
        save();
    });

    // Print
    _actionPrint = new QAction(this);
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
    _actionShowGrid = new QAction("Toggle Grid", this);
    _actionShowGrid->setIcon( QIcon( ":/grid.svg") );
    _actionShowGrid->setCheckable(true);
    _actionShowGrid->setChecked(_settings.showGrid);
    _actionShowGrid->setToolTip("Toggle grid visibility");
    connect(_actionShowGrid, &QAction::toggled, [this](bool checked){
        _settings.showGrid = checked;
        settingsChanged();
    });

    // Fit all
    _actionFitAll = new QAction("Fit All", this);
    _actionFitAll->setIcon(QIcon(":/fit_all.svg"));
    _actionFitAll->setToolTip("Center view on all items");
    connect(_actionFitAll, &QAction::triggered, [this]{_view->fitInView();});

    // Route straight angles
    _actionRouteStraightAngles = new QAction("Wire angles", this);
    _actionRouteStraightAngles->setIcon( QIcon( ":/wire_rightangle.svg") );
    _actionRouteStraightAngles->setCheckable(true);
    _actionRouteStraightAngles->setChecked(_settings.routeStraightAngles);
    connect(_actionRouteStraightAngles, &QAction::toggled, [this](bool checked){
        _settings.routeStraightAngles = checked;
        _settings.preserveStraightAngles = checked;
        settingsChanged();
    });

    // Generate netlist
    _actionGenerateNetlist = new QAction("Generate netlist", this);
    _actionGenerateNetlist->setIcon( QIcon( ":/netlist.svg" ) );
    connect(_actionGenerateNetlist, &QAction::triggered, [this]{
        generateNetlist();
    });

    // Clear
    _actionClear = new QAction("Clear", this);
    _actionClear->setIcon(QIcon(":/clean.svg"));
    connect(_actionClear, &QAction::triggered, [this]{
        Q_ASSERT(_scene);

        _scene->clear();
    });

    // Debug mode
    _actionDebugMode = new QAction("Debug", this);
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

void MainWindow::generateNetlist()
{
    QSchematic::Netlist<Operation*, OperationConnector*> netlist;
    QSchematic::NetlistGenerator::generate(netlist, *_scene);

    Q_ASSERT( _netlistViewerWidget );
    _netlistViewerWidget->setNetlist( netlist );
}

void MainWindow::demo()
{
    _scene->clear();
    _scene->setSceneRect(-500, -500, 3000, 3000);

    load(":/demo_01.xml");

    generateNetlist();
}
