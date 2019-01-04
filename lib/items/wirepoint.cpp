#include "wirepoint.h"

using namespace QSchematic;

WirePoint::WirePoint() :
    QPointF()
{
    _isJunction = false;
}

WirePoint::WirePoint(const WirePoint& other) :
    QPointF(other)
{
    _isJunction = other._isJunction;
}

WirePoint::WirePoint(const QPoint& point) :
    QPointF(point)
{
    _isJunction = false;
}

WirePoint::WirePoint(const QPointF& point) :
    QPointF(point)
{
    _isJunction = false;
}

WirePoint::WirePoint(int x, int y) :
    QPointF(x, y)
{
    _isJunction = false;
}

WirePoint::WirePoint(qreal x, qreal y) :
    QPointF(x, y)
{
    _isJunction = false;
}

QPointF WirePoint::toPointF() const
{
    return QPointF(x(), y());
}

void WirePoint::setIsJunction(bool isJunction)
{
    _isJunction = isJunction;
}

bool WirePoint::isJunction() const
{
    return _isJunction;
}

bool operator==(const WirePoint& a, const WirePoint& b)
{
    return operator==(a.toPoint(), b.toPoint());
}

bool operator==(const WirePoint& a, const QPoint& b)
{
    return operator==(a.toPoint(), b);
}

bool operator==(const WirePoint& a, const QPointF& b)
{
    return operator==(a.toPointF(), b);
}

const QPoint operator+(const WirePoint& a, const QPoint& b)
{
    return operator+(a.toPoint(), b);
}

const QPointF operator+(const WirePoint& a, const QPointF& b)
{
    return operator+(a.toPointF(), b);
}

