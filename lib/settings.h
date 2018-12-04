#pragma once

#include <chrono>

class QPoint;
class QPointF;
class QRectF;
class QVector2D;

namespace QSchematic {

    class Settings
    {
    public:
        Settings();
        virtual ~Settings() = default;

        QPoint toGridPoint(const QPointF& point) const;
        QPoint toScenePoint(const QPoint& gridPoint) const;
        QPoint snapToGridPoint(const QPointF& scenePoint) const;
        void snapToGrid(QVector2D& sceneVector) const;

        bool debug;
        int gridSize;
        int gridPointSize;
        bool showGrid;
        int highlightRectPadding;
        int resizeHandleSize;
        bool routeStraightAngles;
        bool preserveStraightAngles;
        bool antialiasing;
        std::chrono::milliseconds popupDelay;
    };

}
