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

    // Get the rectangle of interest (er = "exposed rect")
    const QRectF er = (option ? option->exposedRect : rect());

    // Prepare painter
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, m_settings.antialiasing);

    // Draw background
    painter->setPen(backgroundPen);
    painter->setBrush(backgroundBrush);
    painter->drawRect(er);

    // Draw the grid if supposed to
    if (m_settings.showGrid && (m_settings.gridSize > 0)) {
        const qreal left = int(er.left()) - (int(er.left()) % m_settings.gridSize);
        const qreal top = int(er.top()) - (int(er.top()) % m_settings.gridSize);

        painter->setPen(gridPen);
        painter->setBrush(gridBrush);
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
