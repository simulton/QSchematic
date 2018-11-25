#pragma once

#include <chrono>

class QPoint;
class QPointF;
class QRectF;
class QPainterPath;

namespace QSchematic {

    class Settings
    {
    public:
        Settings();
        virtual ~Settings() = default;

        QPoint toGridPoint(const QPointF& point) const;
        QPoint toScenePoint(const QPoint& gridPoint) const;
        QPoint snapToGridPoint(const QPointF& scenePoint) const;

        bool debug;
        int gridSize;
        int gridPointSize;
        bool showGrid;
        int highlightRectPadding;
        int resizeHandleSize;
        bool routeStraightAngles;
        bool antialiasing;
        std::chrono::milliseconds popupDelay;
    };

}
