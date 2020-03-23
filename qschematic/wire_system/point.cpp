#include "point.h"

using namespace wire_system;

point::point() :
    QPointF()
{
    m_is_junction = false;
}

point::point(const point& other) :
    QPointF(other)
{
    m_is_junction = other.m_is_junction;
}

point::point(const QPoint& point) :
    QPointF(point)
{
    m_is_junction = false;
}

point::point(const QPointF& point) :
    QPointF(point)
{
    m_is_junction = false;
}

point::point(int x, int y) :
    QPointF(x, y)
{
    m_is_junction = false;
}

point::point(qreal x, qreal y) :
    QPointF(x, y)
{
    m_is_junction = false;
}

QPointF point::toPointF() const
{
    return QPointF(x(), y());
}

void point::set_is_junction(bool isJunction)
{
    m_is_junction = isJunction;
}

bool point::is_junction() const
{
    return m_is_junction;
}

bool operator==(const point& a, const point& b)
{
    return operator==(a.toPoint(), b.toPoint());
}

bool operator==(const point& a, const QPoint& b)
{
    return operator==(a.toPoint(), b);
}

bool operator==(const point& a, const QPointF& b)
{
    return operator==(a.toPointF(), b);
}

const QPoint operator+(const point& a, const QPoint& b)
{
    return operator+(a.toPoint(), b);
}

const QPointF operator+(const point& a, const QPointF& b)
{
    return operator+(a.toPointF(), b);
}

