#pragma once

#include <qschematic-export.h>

#include <chrono>

class QPoint;
class QPointF;
class QRectF;
class QVector2D;
class QSize;
class QSizeF;

namespace QSchematic
{

    /**
     * Settings for the QSchematic scene.
     */
    class QSCHEMATIC_EXPORT Settings
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

        [[nodiscard]]
        QPoint
        toGridPoint(const QPointF& point) const;

        [[nodiscard]]
        QPoint
        toScenePoint(const QPoint& gridPoint) const;

        [[nodiscard]]
        QPoint
        snapToGrid(const QPointF& scenePoint) const;

        [[nodiscard]]
        QVector2D
        snapToGrid(const QVector2D& sceneVector) const;

        [[nodiscard]]
        QSize
        snapToGrid(const QSizeF& sceneSize) const;
    };

}
