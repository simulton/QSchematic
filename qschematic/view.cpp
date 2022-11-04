#include <QKeyEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QtMath>

#include "view.h"
#include "scene.h"
#include "settings.h"
#include "commands/commanditemremove.h"

const qreal ZOOM_FACTOR_MIN   = 0.25;
const qreal ZOOM_FACTOR_MAX   = 10.00;
const qreal ZOOM_FACTOR_STEPS = 0.10;
const qreal FIT_ALL_PADDING   = 20.00;

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
            return;

        case Qt::Key_Minus:
            _scaleFactor -= ZOOM_FACTOR_STEPS;
            updateScale();
            return;

        case Qt::Key_0:
            setZoomValue(1.0);
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
            if (_scene->mode() == Scene::NormalMode) {
                for (auto item : _scene->selectedTopLevelItems()) {
                    _scene->undoStack()->push(new CommandItemRemove(_scene, item));
                }
            } else {
                _scene->removeLastWirePoint();
            }
        }
        return;

    case Qt::Key_Backspace:
        if (_scene && _scene->mode() == Scene::WireMode)
            _scene->removeLastWirePoint();
        else
            QGraphicsView::keyPressEvent(event);

        return;

    default:
        break;
    }

    // Fall back
    QGraphicsView::keyPressEvent(event);
}

void View::wheelEvent(QWheelEvent* event)
{
    // CTRL + wheel to zoom
    if (event->modifiers() & Qt::ControlModifier) {

        // Zoom in (clip)
        if (event->angleDelta().y() > 0) {
            _scaleFactor += ZOOM_FACTOR_STEPS;
        }

        // Zoom out (clip)
        else if (event->angleDelta().y() < 0) {
            _scaleFactor -= ZOOM_FACTOR_STEPS;
        }

        _scaleFactor = qBound(0.0, _scaleFactor, 1.0);

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
        connect(scene, &Scene::modeChanged, [this](int newMode){
            switch (newMode) {
            case Scene::NormalMode:
                viewport()->setCursor(Qt::ArrowCursor);
                break;

            case Scene::WireMode:
                viewport()->setCursor(Qt::CrossCursor);
                break;

            default:
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
    _scaleFactor = qLn(ZOOM_FACTOR_MIN/factor) / qLn(ZOOM_FACTOR_MIN / ZOOM_FACTOR_MAX);

    updateScale();
}

void View::updateScale()
{
    // Exponential interpolation
    float logMinZoom = qLn(ZOOM_FACTOR_MIN);
    float logMaxZoom = qLn(ZOOM_FACTOR_MAX);
    float logZoom = logMinZoom + (logMaxZoom - logMinZoom) * _scaleFactor;
    float zoom = qExp(logZoom);

    // Apply the new scale
    setTransform(QTransform::fromScale(zoom, zoom));

    emit zoomChanged(zoom);
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

void View::fitInView()
{
    // Check if there is a scene
    if (!_scene) {
        return;
    }

    // Find the combined bounding rect of all the items
    QRectF rect;
    for (const auto& item : _scene->QGraphicsScene::items()) {
        QRectF boundingRect = item->boundingRect();
        boundingRect.moveTo(item->scenePos());
        rect = rect.united(boundingRect);
    }

    // Add some padding
    const auto& adj = std::max(0.0, FIT_ALL_PADDING);
    rect.adjust(-adj, -adj, adj, adj);

    // Update and cap the scale factor
    qreal currentScaleFactor = _scaleFactor;
    qreal newScaleFactor = _scaleFactor;
    QGraphicsView::fitInView(rect, Qt::KeepAspectRatio);
    newScaleFactor = viewport()->geometry().width() / mapToScene(viewport()->geometry()).boundingRect().width();
    if (currentScaleFactor < 1)
        newScaleFactor = std::min(newScaleFactor, 1.0);
    else
        newScaleFactor = std::min(newScaleFactor, currentScaleFactor);
    setZoomValue(newScaleFactor);
}
