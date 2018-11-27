#include <QPainter>
#include "operation.h"
#include "myconnector.h"

const QColor COLOR_HIGHLIGHTED = QColor(Qt::blue).lighter();
const QColor COLOR_BODY_FILL   = QColor(Qt::gray).lighter(140);
const QColor COLOR_BODY_BORDER = QColor(Qt::black);
const qreal PEN_WIDTH          = 1.5;

Operation::Operation(QGraphicsItem* parent) :
    QSchematic::Node(parent)
{
    setSize(10, 5);
    setAllowMouseResize(false);
    setConnectorsMovable(false);
    setConnectorsSnapPolicy(QSchematic::Connector::Anywhere);
    setConnectorsSnapToGrid(true);

    // Add connectors
    addConnector(new MyConnector(QPoint(0, 3)));
    addConnector(new MyConnector(QPoint(10, 3)));
}

void Operation::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    // Draw the bounding rect if debug mode is enabled
    if (_settings.debug) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(Qt::red));
        painter->drawRect(boundingRect());
    }

    // Highlight rectangle
    if (isHighlighted()) {
        // Highlight pen
        QPen highlightPen;
        highlightPen.setStyle(Qt::NoPen);

        // Highlight brush
        QBrush highlightBrush;
        highlightBrush.setStyle(Qt::SolidPattern);
        highlightBrush.setColor(COLOR_HIGHLIGHTED);

        // Highlight rectangle
        painter->setPen(highlightPen);
        painter->setBrush(highlightBrush);
        painter->setOpacity(0.5);
        int adj = _settings.highlightRectPadding;
        painter->drawRoundedRect(QRect(QPoint(0, 0), size()*_settings.gridSize).adjusted(-adj, -adj, adj, adj), _settings.gridSize/2, _settings.gridSize/2);
    }

    painter->setOpacity(1.0);

    // Body pen
    QPen bodyPen;
    bodyPen.setWidthF(PEN_WIDTH);
    bodyPen.setStyle(Qt::SolidLine);
    bodyPen.setColor(COLOR_BODY_BORDER);

    // Body brush
    QBrush bodyBrush;
    bodyBrush.setStyle(Qt::SolidPattern);
    bodyBrush.setColor(COLOR_BODY_FILL);

    // Draw the component body
    painter->setPen(bodyPen);
    painter->setBrush(bodyBrush);
    painter->drawRoundedRect(QRect(QPoint(0, 0), size()*_settings.gridSize), _settings.gridSize/2, _settings.gridSize/2);

    // Resize handles
    if (isSelected() and allowMouseResize()) {
        for (const QRect& rect : resizeHandles()) {
            // Handle pen
            QPen handlePen;
            handlePen.setStyle(Qt::NoPen);
            painter->setPen(handlePen);

            // Handle Brush
            QBrush handleBrush;
            handleBrush.setStyle(Qt::SolidPattern);
            painter->setBrush(handleBrush);

            // Draw the outer handle
            handleBrush.setColor("#3fa9f5");
            painter->setBrush(handleBrush);
            painter->drawRect(rect.adjusted(-handlePen.width(), -handlePen.width(), handlePen.width()/2, handlePen.width()/2));

            // Draw the inner handle
            int adj = _settings.resizeHandleSize/2;
            handleBrush.setColor(Qt::white);
            painter->setBrush(handleBrush);
            painter->drawRect(rect.adjusted(-handlePen.width()+adj, -handlePen.width()+adj, (handlePen.width()/2)-adj, (handlePen.width()/2)-adj));
        }
    }
}
