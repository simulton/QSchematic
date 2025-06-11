#pragma once

#include "point.hpp"

#include <QList>
#include <QVector>

#include <memory>

class QVector2D;

namespace wire_system
{
    class manager;
    class net;
    class line;

    /**
     * A wire to connect to connectables and other wires.
     *
     * @details A wire consists of zero or more line segments. A line segment's point can connect to connectables or
     *          other wire segments.
     */
    class wire
    {
    public:
        wire() = default;
        wire(const wire&) = delete;
        wire(wire&&) = delete;
        virtual ~wire() = default;

        void
        set_manager(manager* manager);

        [[nodiscard]]
        QVector<point>
        points() const;

        [[nodiscard]]
        int
        points_count() const;

        [[nodiscard]]
        QVector<int>
        junctions() const;

        [[nodiscard]]
        QList<wire*>
        connected_wires();

        [[nodiscard]]
        QList<line>
        line_segments() const;

        virtual
        void
        move_point_to(int index, const QPointF& moveTo);

        void
        set_point_is_junction(int index, bool isJunction);

        virtual
        void
        prepend_point(const QPointF& point);

        virtual
        void
        append_point(const QPointF& point);

        virtual
        void
        insert_point(int index, const QPointF& point);

        void
        move_point_by(int index, const QVector2D& moveBy);

        [[nodiscard]]
        bool
        point_is_on_wire(const QPointF& point) const;

        void
        move(const QVector2D& movedBy);

        void
        simplify();

        [[nodiscard]]
        bool
        connect_wire(wire* wire);

        void
        setNet(const std::shared_ptr<wire_system::net>& net);

        [[nodiscard]]
        std::shared_ptr<wire_system::net>
        net();

        void
        disconnectWire(wire* wire);

        virtual
        void
        add_segment(int index);

        void
        remove_point(int index);

        [[nodiscard]]
        class manager*
        manager() noexcept
        {
            return m_manager;
        }

    protected:
        QVector<point> m_points;

        void
        move_junctions_to_new_segment(const line& oldSegment, const line& newSegment);

        void
        move_line_segment_by(int index, const QVector2D& moveBy);

    private:
        QList<wire*> m_connectedWires;
        std::shared_ptr<wire_system::net> m_net;
        class manager* m_manager = nullptr;

        void
        remove_duplicate_points();

        void
        remove_obsolete_points();

        virtual
        void
        about_to_change();

        virtual
        void
        has_changed();
    };
}
