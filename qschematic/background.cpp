#include "background.hpp"

#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

using namespace QSchematic;

Background::Background(QGraphicsItem* parent) :
    QGraphicsRectItem(parent)
{
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);  // For QStyleOptionGraphicsItem::exposedRect
}

void
Background::setSettings(const Settings& settings)
{
    m_settings = settings;
}

void
Background::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // ToDo: Use QStyleOptionGraphicsItem::exposedRect

    // Background pen
    QPen backgroundPen;
    backgroundPen.setStyle(Qt::NoPen);

    // Background brush
    QBrush backgroundBrush;
    backgroundBrush.setStyle(Qt::SolidPattern);
    backgroundBrush.setColor(Qt::white);

    // Grid pen
    QPen gridPen;
    gridPen.setStyle(Qt::SolidLine);
    gridPen.setColor(Qt::gray);
    gridPen.setCapStyle(Qt::RoundCap);
    gridPen.setWidth(m_settings.gridPointSize);

    // Grid brush
    QBrush gridBrush;
    gridBrush.setStyle(Qt::NoBrush);

    // Get the rects
    //
    // sr = scene rect
    // ep = exposed rect
    const QRectF sr = rect();
    const QRectF er = (option ? option->exposedRect : rect());

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, m_settings.antialiasing);

    // Draw background
    painter->setPen(backgroundPen);
    painter->setBrush(backgroundBrush);
    painter->drawRect(rect());

    // Draw the grid if supposed to
    if (m_settings.showGrid && (m_settings.gridSize > 0)) {
        qreal left = int(rect().left()) - (int(rect().left()) % m_settings.gridSize);
        qreal top = int(rect().top()) - (int(rect().top()) % m_settings.gridSize);

        painter->setPen(gridPen);
        painter->setBrush(gridBrush);
        for (qreal x = left; x < sr.right(); x += m_settings.gridSize) {
            for (qreal y = top; y < sr.bottom(); y += m_settings.gridSize)
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
