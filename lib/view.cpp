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
    _scaleFactor(1.0),
    _pan(false),
    _panStartX(0),
    _panStartY(0)
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
    // Zoom
    if (event->modifiers() & Qt::ControlModifier) {

        // Zoom in
        if (event->key() == Qt::Key_Plus) {
            _scaleFactor += ZOOM_FACTOR_STEPS;
            updateScale();
        }

        // Zoom out
        if (event->key() == Qt::Key_Minus) {
            _scaleFactor -= ZOOM_FACTOR_STEPS;
            updateScale();
        }

        // Reset zoom
        if (event->key() == Qt::Key_0) {
            _scaleFactor = 1.0;
            updateScale();
        }
    } else {
        QGraphicsView::keyPressEvent(event);
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

    if (_pan) {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (event->x() - _panStartX));
        verticalScrollBar()->setValue(verticalScrollBar()->value() - (event->y() - _panStartY));
        _panStartX = event->x();
        _panStartY = event->y();
        event->accept();
        return;
    }
}

void View::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);

    if (event->button() == Qt::MiddleButton) {
        _pan = true;
        _panStartX = event->x();
        _panStartY = event->y();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);

    if (event->button() == Qt::MiddleButton) {
        _pan = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
}

void View::setSettings(const Settings& settings)
{
    _settings = settings;

    // Rendering options
    setRenderHint(QPainter::Antialiasing, _settings.antialiasing);
}

void View::setMode(Scene::Mode mode)
{
#warning ToDo: Remove this and exclusively use the Scene::proposedCursor() signal
    // Set the cursor shape
    switch (mode) {
    case Scene::NormalMode:
        viewport()->setCursor(Qt::ArrowCursor);
        break;

    case Scene::WireMode:
        viewport()->setCursor(Qt::CrossCursor);
        break;
    }

    update();
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

qreal View::zoomValue() const
{
    return _scaleFactor;
}
