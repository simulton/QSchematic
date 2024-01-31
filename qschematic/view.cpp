#include <QKeyEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QtMath>

#include "view.hpp"
#include "scene.hpp"
#include "settings.hpp"
#include "commands/item_remove.hpp"

using namespace QSchematic;

View::View(QWidget* parent) :
    QGraphicsView(parent)
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

void
View::keyPressEvent(QKeyEvent* event)
{
    // Something with CTRL held down?
    if (event->modifiers() & Qt::ControlModifier) {

        switch (event->key()) {
            case Qt::Key_Plus:
                _scaleFactor += zoom_factor_step;
                updateScale();
                return;

            case Qt::Key_Minus:
                _scaleFactor -= zoom_factor_step;
                updateScale();
                return;

            case Qt::Key_0:
                setZoomValue(1.0);
                updateScale();
                return;

            case Qt::Key_W:
                if (_scene)
                    _scene->setMode(Scene::WireMode);
                return;

            case Qt::Key_Space:
                if (_scene)
                    _scene->toggleWirePosture();
                return;

            default:
                break;
        }
    }

    // Just a key alone?
    switch (event->key()) {
        case Qt::Key_Escape:
            if (_scene)
                _scene->setMode(Scene::NormalMode);
            return;

        case Qt::Key_Delete:
            if (_scene) {
                if (_scene->mode() == Scene::NormalMode) {
                    for (auto item : _scene->selectedTopLevelItems())
                        _scene->undoStack()->push(new Commands::ItemRemove(_scene, item));
                }
                else
                    _scene->removeLastWirePoint();
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

void
View::wheelEvent(QWheelEvent* event)
{
    // CTRL + wheel to zoom
    if (event->modifiers() & Qt::ControlModifier) {

        // Zoom in (clip)
        if (event->angleDelta().y() > 0)
            _scaleFactor += zoom_factor_step;

        // Zoom out (clip)
        else if (event->angleDelta().y() < 0)
            _scaleFactor -= zoom_factor_step;

        _scaleFactor = qBound(0.0, _scaleFactor, 1.0);

        updateScale();
    }
}

void
View::mouseMoveEvent(QMouseEvent* event)
{
    QGraphicsView::mouseMoveEvent(event);

    switch (_mode) {
        case NormalMode:
            break;

        case PanMode:
#       if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (event->position().x() - _panStart.x()));
            verticalScrollBar()->setValue(verticalScrollBar()->value() - (event->position().y() - _panStart.y()));
#       else
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (event->x() - _panStart.x()));
            verticalScrollBar()->setValue(verticalScrollBar()->value() - (event->y() - _panStart.y()));
#       endif
            _panStart = event->pos();
            event->accept();
            return;
    }
}

void
View::mousePressEvent(QMouseEvent* event)
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

void
View::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        setMode(NormalMode);
        viewport()->setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void
View::setScene(Scene* scene)
{
    if (scene) {
        // Change cursor depending on scene mode
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

void
View::setSettings(const Settings& settings)
{
    _settings = settings;

    // Rendering options
    setRenderHint(QPainter::Antialiasing, _settings.antialiasing);
}

void
View::setZoomValue(qreal factor)
{
    _scaleFactor = qLn(zoom_factor_min/factor) / qLn(zoom_factor_min / zoom_factor_max);

    updateScale();
}

void
View::updateScale()
{
    // Exponential interpolation
    float logMinZoom = qLn(zoom_factor_min);
    float logMaxZoom = qLn(zoom_factor_max);
    float logZoom = logMinZoom + (logMaxZoom - logMinZoom) * _scaleFactor;
    float zoom = qExp(logZoom);

    // Apply the new scale
    setTransform(QTransform::fromScale(zoom, zoom));

    emit zoomChanged(zoom);
}

void
View::setMode(const Mode newMode)
{
    _mode = newMode;

    emit modeChanged(_mode);
}

qreal
View::zoomValue() const
{
    return _scaleFactor;
}

void
View::fitInView()
{
    // Sanity check
    if (!_scene)
        return;

    // Find the combined bounding rect of all the items
    QRectF rect;
    for (const auto& item : _scene->QGraphicsScene::items()) {
        QRectF boundingRect = item->boundingRect();
        boundingRect.moveTo(item->scenePos());
        rect = rect.united(boundingRect);
    }

    // Add some padding
    const auto adj = std::max(0.0, fitall_padding);
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
