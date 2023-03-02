#pragma once

#include "types.h"

#include <QVector>

class QPoint;
class QPointF;
class QLineF;
class QRectF;
class QPainterPath;

namespace QSchematic
{

    class Utils
    {
    public:
        enum RectanglePointTypes {
            RectangleCornerPoints     = 0x01,
            RectangleEdgeCenterPoints = 0x02
        };

        static QPoint centerPoint(const QPoint& p1, const QPoint& p2);
        static QPointF centerPoint(const QPointF& p1, const QPointF& p2);
        static QPointF clipPointToRect(QPointF point, const QRectF& rect);
        static QPointF clipPointToRectOutline(QPointF point, const QRectF& rect);
        static QPointF pointOnLineClosestToPoint(const QPointF& p1, const QPointF& p2, const QPointF& point);
        static QVector<QLineF>::const_iterator lineClosestToPoint(const QVector<QLineF>& lines, const QPointF& point);
        static QVector<QPointF> rectanglePoints(const QRectF& rect, RectanglePointTypes pointTypes);
        static bool lineIsHorizontal(const QPointF& p1, const QPointF& p2);
        static bool lineIsVertical(const QPointF& p1, const QPointF& p2);
        static bool pointIsOnLine(const QLineF& line, const QPointF& point);

        /**
         * Returns a list of line segments constructed from the provided points.
         *
         * @note If `closeLoop` is set to `true`, a line connecting the first and last point will be appended.
         * @note If points does not hold at least two elements, an empty container will be returned.
         */
        [[nodiscard]]
        static
        std::vector<QLineF>
        linesFromPoints(const QVector<QPointF>& points, bool closeLoop);
    private:
        Utils() = default;
        Utils(const Utils& other) = default;
        virtual ~Utils() = default;
        Utils& operator=(const Utils& other) = default;
    };

}
