#include <QKeyEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QAction>
#include <QMenu>
#include "view.h"
#include "scene.h"
#include "settings.h"

const qreal ZOOM_FACTOR_MIN  = 0.25;
const qreal ZOOM_FACTOR_MAX   = 2.00;
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
    setDragMode(QGraphicsView::RubberBandDrag);

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
            break;

        case Qt::Key_Minus:
            _scaleFactor -= ZOOM_FACTOR_STEPS;
            updateScale();
            break;

        case Qt::Key_0:
            _scaleFactor = 1.0;
            updateScale();
            break;

        case Qt::Key_W:
            if (_scene) {
                _scene->setMode(Scene::WireMode);
            }
            break;

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
        break;

    default:
        break;
    }
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
    QGraphicsView::mousePressEvent(event);

    if (event->button() == Qt::MiddleButton) {
        setMode(PanMode);
        _panStart = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);

    if (event->button() == Qt::MiddleButton) {
        setMode(NormalMode);
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
}

void View::setScene(Scene* scene)
{
    if (scene) {
        connect(scene, &Scene::modeChanged, this, &View::sceneModeChanged);
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

void View::sceneModeChanged(Scene::Mode newMode)
{
#warning ToDo: Remove this and exclusively use the Scene::proposedCursor() signal
    // Set the cursor shape
    switch (newMode) {
    case Scene::NormalMode:
        viewport()->setCursor(Qt::ArrowCursor);
        break;

    case Scene::WireMode:
        viewport()->setCursor(Qt::CrossCursor);
        break;
    }
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
