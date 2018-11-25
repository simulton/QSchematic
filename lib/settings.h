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

        static QPoint centerPoint(const QPoint& p1, const QPoint& p2);
        static QPointF clipPointToRect(QPointF point, const QRectF& rect);
        static QPointF clipPointToRectOutline(QPointF point, const QRectF& rect);
        static QPointF clipPointToPath(const QPointF& point, const QPainterPath& path);

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
