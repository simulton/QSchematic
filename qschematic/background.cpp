#include "background.hpp"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

using namespace QSchematic;

Background::Background(QGraphicsItem* parent) :
    QGraphicsRectItem(parent)
{
    // Background pen
    m_background_pen.setStyle(Qt::NoPen);

    // Background brush
    m_background_brush.setStyle(Qt::SolidPattern);
    m_background_brush.setColor(Qt::white);

    // Grid pen
    m_grid_pen.setStyle(Qt::SolidLine);
    m_grid_pen.setColor(Qt::gray);
    m_grid_pen.setCapStyle(Qt::RoundCap);
    m_grid_pen.setWidth(m_settings.gridPointSize);

    // Grid brush
    m_grid_brush.setStyle(Qt::NoBrush);

    // Configuration
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);  // For QStyleOptionGraphicsItem::exposedRect
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsFocusable, false);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, false);
}

void
Background::setSettings(const Settings& settings)
{
    m_settings = settings;
}

void
Background::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // Get the rectangle of interest (er = "exposed rect")
    const QRectF er = (option ? option->exposedRect : rect());

    // Prepare painter
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, m_settings.antialiasing);

    // Draw background
    painter->setPen(m_background_pen);
    painter->setBrush(m_background_brush);
    painter->drawRect(er);

    // Draw the grid if supposed to
    if (m_settings.showGrid && (m_settings.gridSize > 0)) {
        const qreal left = int(er.left()) - (int(er.left()) % m_settings.gridSize);
        const qreal top = int(er.top()) - (int(er.top()) % m_settings.gridSize);

        painter->setPen(m_grid_pen);
        painter->setBrush(m_grid_brush);
        for (qreal x = left; x < er.right(); x += m_settings.gridSize) {
            for (qreal y = top; y < er.bottom(); y += m_settings.gridSize)
                painter->drawPoint(x, y);
        }
    }

    // Mark the origin if supposed to
    if (m_settings.debug) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(Qt::red));
        painter->drawEllipse(-6, -6, 12, 12);
    }

    painter->restore();
}
