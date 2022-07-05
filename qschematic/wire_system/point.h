#pragma once

#include <QPointF>

namespace wire_system {

    class point :
        private QPointF
    {
    public:
        // Expose some of the QPoint interfaces
        using QPointF::setX;
        using QPointF::setY;
        using QPointF::x;
        using QPointF::y;
        using QPointF::rx;
        using QPointF::ry;
        using QPointF::toPoint;

        point();
        point(const point& other);
        point(point&&) = default;
        point(const QPoint& point);
        point(const QPointF& point);
        point(int x, int y);
        point(qreal x, qreal y);
        virtual ~point() = default;
        point& operator=(const point&) = default;

        QPointF toPointF() const;
        void set_is_junction(bool isJunction);
        [[nodiscard]] bool is_junction() const;

    private:
        bool m_is_junction;
    };
}

bool operator==(const wire_system::point& a, const wire_system::point& b);
bool operator==(const wire_system::point& a, const QPoint& b);
bool operator==(const wire_system::point& a, const QPointF& b);
const QPoint operator+(const wire_system::point& a, const QPoint& b);
const QPointF operator+(const wire_system::point& a, const QPointF& b);
