#include <QKeyEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QAction>
#include <QMenu>
#include "commands/commanditemremove.h"
#include "view.h"
#include "scene.h"
#include "settings.h"

const qreal ZOOM_FACTOR_MIN   = 0.25;
const qreal ZOOM_FACTOR_MAX   = 10.00;
const qreal ZOOM_FACTOR_STEPS = 0.10;

using namespace QSchematic;

View::View(QWidget* parent) :
    QGraphicsView(parent),
    _scene(nullptr),
    _scaleFactor(1.0),
    _mode(NormalMode)
{
    // Scroll bars
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // Interaction stuff
    setMouseTracking(true);
    setAcceptDrops(true);
    setDragMode(QGraphicsView::NoDrag);

    // Rendering options
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
}

void View::keyPressEvent(QKeyEvent* event)
{
    // Something with CTRL held down?
    if (event->modifiers() & Qt::ControlModifier) {

        switch (event->key()) {
        case Qt::Key_Plus:
            _scaleFactor += ZOOM_FACTOR_STEPS;
            updateScale();
            return;

        case Qt::Key_Minus:
            _scaleFactor -= ZOOM_FACTOR_STEPS;
            updateScale();
            return;

        case Qt::Key_0:
            _scaleFactor = 1.0;
            updateScale();
            return;

        case Qt::Key_W:
            if (_scene) {
                _scene->setMode(Scene::WireMode);
            }
            return;

        case Qt::Key_Space:
            if (_scene) {
                _scene->toggleWirePosture();
            }
            return;

        default:
            break;
        }
    }

    // Just a key alone?
    switch (event->key()) {
    case Qt::Key_Escape:
        if (_scene) {
            _scene->setMode(Scene::NormalMode);
        }
        return;

    case Qt::Key_Delete:
        if (_scene) {
            for (auto item : _scene->selectedItems()) {
                auto qschematicItem = dynamic_cast<Item*>(item);
                if (qschematicItem) {
                    _scene->undoStack()->push(new CommandItemRemove(_scene, qschematicItem));
                }
            }
        }
        return;

    default:
        break;
    }

    // Fall back
    QGraphicsView::keyPressEvent(event);
}

void View::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->angleDelta().y() > 0 && _scaleFactor < ZOOM_FACTOR_MAX) {
            _scaleFactor += ZOOM_FACTOR_STEPS;
        } else if (event->angleDelta().y() < 0 && _scaleFactor > ZOOM_FACTOR_MIN) {
            _scaleFactor -= ZOOM_FACTOR_STEPS;
        }

        updateScale();
    }
}

void View::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);

    switch (_mode) {
    case NormalMode:
        break;

    case PanMode:
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (event->x() - _panStart.x()));
        verticalScrollBar()->setValue(verticalScrollBar()->value() - (event->y() - _panStart.y()));
        _panStart = event->pos();
        event->accept();
        return;
    }
}

void View::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        setMode(PanMode);
        _panStart = event->pos();
        viewport()->setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        setMode(NormalMode);
        viewport()->setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void View::setScene(Scene* scene)
{
    if (scene) {
        connect(scene, &Scene::modeChanged, [this](Scene::Mode newMode){
            switch (newMode) {
            case Scene::NormalMode:
                viewport()->setCursor(Qt::ArrowCursor);
                break;

            case Scene::WireMode:
                viewport()->setCursor(Qt::CrossCursor);
                break;
            }
        });
    }

    QGraphicsView::setScene(scene);

    _scene = scene;
}

void View::setSettings(const Settings& settings)
{
    _settings = settings;

    // Rendering options
    setRenderHint(QPainter::Antialiasing, _settings.antialiasing);
}

void View::setZoomValue(qreal factor)
{
    _scaleFactor = factor;

    updateScale();
}

void View::updateScale()
{
    // Apply the new scale
    setTransform(QTransform::fromScale(_scaleFactor, _scaleFactor));

    emit zoomChanged(_scaleFactor);
}

void View::setMode(Mode newMode)
{
    _mode = newMode;

    emit modeChanged(_mode);
}

qreal View::zoomValue() const
{
    return _scaleFactor;
}
