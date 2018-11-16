#pragma once

#include <QPoint>

namespace QSchematic {

    class WirePoint : private QPoint
    {
    public:
        // Expose some of the QPoint interfaces
        using QPoint::setX;
        using QPoint::setY;
        using QPoint::x;
        using QPoint::y;

        WirePoint();
        WirePoint(const WirePoint& other);
        WirePoint(const QPoint& point);
        WirePoint(int x, int y);
        virtual ~WirePoint() = default;

        QPoint toPoint() const;
        void setIsJunction(bool isJunction);
        bool isJunction() const;

    private:
        bool _isJunction;
    };
}

bool operator==(const QSchematic::WirePoint& a, const QSchematic::WirePoint& b);
bool operator==(const QSchematic::WirePoint& a, const QPoint& b);
const QPoint operator+(const QSchematic::WirePoint& a, const QPoint& b);
