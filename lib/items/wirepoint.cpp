#include "wirepoint.h"

using namespace QSchematic;

WirePoint::WirePoint() :
    QPoint()
{
    _isJunction = false;
}

WirePoint::WirePoint(const WirePoint& other) :
    QPoint(other)
{
    _isJunction = other._isJunction;
}

WirePoint::WirePoint(const QPoint& point) :
    QPoint(point)
{
    _isJunction = false;
}

WirePoint::WirePoint(int x, int y) :
    QPoint(x, y)
{
    _isJunction = false;
}

QPoint WirePoint::toPoint() const
{
    return QPoint(x(), y());
}

void WirePoint::setIsJunction(bool isJunction)
{
    _isJunction = isJunction;
}

bool WirePoint::isJunction() const
{
    return _isJunction;
}

bool operator==(const WirePoint& a, const WirePoint& b) {
    return operator==(a.toPoint(), b.toPoint());
}

bool operator==(const WirePoint& a, const QPoint& b) {
    return operator==(a.toPoint(), b);
}

const QPoint operator+(const WirePoint& a, const QPoint& b) {
    return operator+(a.toPoint(), b);
}

