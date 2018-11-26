#pragma once

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
        static QPoint centerPoint(const QPoint& p1, const QPoint& p2);
        static QPointF clipPointToRect(QPointF point, const QRectF& rect);
        static QPointF clipPointToRectOutline(QPointF point, const QRectF& rect);
        static QPointF clipPointToPath(const QPointF& point, const QPainterPath& path);
        static QPointF pointOnLineClosestToPoint(const QPointF& p1, const QPointF& p2, const QPointF& point);
        static QVector<QLineF>::const_iterator lineClosestToPoint(const QVector<QLineF>& lines, const QPointF& point);

    private:
        Utils() = default;
        Utils(const Utils& other) = default;
        virtual ~Utils() = default;
        Utils& operator=(const Utils& other) = default;
    };

}
