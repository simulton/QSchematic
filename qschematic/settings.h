#pragma once

#include <chrono>

class QPoint;
class QPointF;
class QRectF;
class QVector2D;
class QSize;
class QSizeF;

namespace QSchematic {

    class Settings
    {
    public:
        bool debug                  = false;
        int gridSize                = 20;
        int gridPointSize           = 3;
        bool showGrid               = true;
        int highlightRectPadding    = 10;
        int resizeHandleSize        = 7;
        bool routeStraightAngles    = true;
        bool preserveStraightAngles = true;
        bool antialiasing           = true;
        std::chrono::milliseconds popupDelay{ 400 };

        // Construction
        Settings() = default;
        Settings(const Settings& other) = default;
        Settings(Settings&& other) = delete;
        virtual ~Settings() = default;

        // Operators
        Settings& operator=(const Settings& rhs) = default;
        Settings& operator=(Settings&& rhs) = delete;

        // Generic
        QPoint toGridPoint(const QPointF& point) const;
        QPoint toScenePoint(const QPoint& gridPoint) const;
        QPoint snapToGrid(const QPointF& scenePoint) const;
        QVector2D snapToGrid(const QVector2D& sceneVector) const;
        QSize snapToGrid(const QSizeF& sceneSize) const;
    };

}
