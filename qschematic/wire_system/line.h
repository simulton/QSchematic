#pragma once

#include <QPointF>

class QLineF;

namespace wire_system {

    class line
    {
    public:
        line() = default;
        line(int x1, int y1, int x2, int y2);
        line(qreal x1, qreal y1, qreal x2, qreal y2);
        line(const QPoint& p1, const QPoint& p2);
        line(const QPointF& p1, const QPointF& p2);
        line(const line&) = default;
        line(line&&) = default;
        virtual ~line() = default;
        line& operator=(const line&) = default;

        [[nodiscard]] QPointF p1() const;
        [[nodiscard]] QPointF p2() const;
        [[nodiscard]] bool is_null() const;
        [[nodiscard]] bool is_horizontal() const;
        [[nodiscard]] bool is_vertical() const;
        [[nodiscard]] qreal lenght() const;
        [[nodiscard]] QPointF mid_point() const;
        [[nodiscard]] bool contains_point(const QPointF& point, qreal tolerance = 0) const;
        [[nodiscard]] QPointF point_on_line_closest_to(const QPointF& point);
        [[nodiscard]] QLineF toLineF() const;

        static bool contains_point(const QLineF& line, const QPointF& point, qreal tolerance = 0);

    private:
        QPointF m_p1;
        QPointF m_p2;
    };

}
