#include "wire.h"
#include "line.h"
#include "net.h"
#include "manager.h"
#include "../utils.h"

#include <QVector2D>
#include <QLineF>

using namespace wire_system;

wire::wire() : m_manager(nullptr)
{
}

void wire::set_manager(wire_system::manager* manager)
{
    m_manager = manager;
}

QVector<point> wire::points() const
{
    return m_points;
}

int wire::points_count() const
{
    return m_points.count();
}

QVector<int> wire::junctions() const
{
    if (points_count() < 2) {
        return {};
    }
    QVector<int> indexes;
    if (m_points.first().is_junction()) {
        indexes.append(0);
    }
    if (m_points.last().is_junction()) {
        indexes.append(points_count() - 1);
    }
    return indexes;
}

QList<wire*> wire::connected_wires()
{
    return m_connectedWires;
}

QList<line> wire::line_segments() const
{
    // A line segment requires at least two points... duuuh
    if (points_count() < 2) {
        return QList<line>();
    }

    QList<line> ret;
    for (int i = 0; i < points_count() - 1; i++) {
        ret.append(line(m_points.at(i).toPointF(), m_points.at(i + 1).toPointF()));
    }

    return ret;
}

void wire::move_junctions_to_new_segment(const line& oldSegment, const line& newSegment)
{
    // Do nothing if the segment was just resized
    if (qFuzzyCompare(oldSegment.toLineF().angle(), newSegment.toLineF().angle())) {
        return;
    }

    // Move connected junctions
    for (const auto& wire: m_connectedWires) {
        for (const auto& jIndex: wire->junctions()) {
            point point = wire->points().at(jIndex);
            // Check if the point is on the old segment
            if (oldSegment.contains_point(point.toPoint(), 5)) {
                line junctionSeg;
                // Find out if one of the segments is horizontal or vertical
                if (jIndex < wire->points().count() - 1) {
                    line seg = wire->line_segments().at(jIndex);
                    if (seg.is_horizontal() || seg.is_vertical()) {
                        junctionSeg = seg;
                    }
                }
                if (jIndex > 0) {
                    line seg = wire->line_segments().at(jIndex - 1);
                    if (seg.is_horizontal() || seg.is_vertical()) {
                        junctionSeg = seg;
                    }
                }
                // Only move in the direction of the segment if it is hor. or vert.
                if (!junctionSeg.is_null()) {
                    QPointF intersection;
#                   if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                        auto type = junctionSeg.toLineF().intersects(newSegment.toLineF(), &intersection);
#                   else
                        auto type = junctionSeg.toLineF().intersect(newSegment.toLineF(), &intersection);
#                   endif
                    if (type != QLineF::NoIntersection) {
                        wire->move_point_by(jIndex, QVector2D(intersection - point.toPointF()));
                    }
                }
                    // Move the point along the segment so that it stays at the same proportional distance from the two points
                else {
                    QPointF d = point.toPointF() - oldSegment.p1();
                    qreal ratio = QVector2D(d).length() / oldSegment.lenght();
                    QPointF pos = newSegment.toLineF().pointAt(ratio);
                    wire->move_point_by(jIndex, QVector2D(pos - point.toPointF()));
                }
            }
        }
    }
}

void wire::move_point_to(int index, const QPointF& moveTo)
{
    if (index < 0 || index > points_count() - 1) {
        return;
    }

    // Do nothing if it already is at that position
    if (points().at(index) == moveTo) {
        return;
    }

    // Move junctions that are on the point
    for (const auto& wire: m_connectedWires) {
        for (const auto& jIndex: wire->junctions()) {
            point point = wire->points().at(jIndex);
            if ((m_points[index]).toPoint() == point.toPoint()) {
                wire->move_point_by(jIndex, QVector2D(moveTo - m_points[index].toPointF()));
            }
        }
    }

    // Move junctions on the next segment
    if (index < points_count() - 1) {
        line segment = line_segments().at(index);
        line newSegment(moveTo, points().at(index + 1).toPointF());
        move_junctions_to_new_segment(segment, newSegment);
    }

    // Move junctions on the previous segment
    if (index > 0) {
        line segment = line_segments().at(index - 1);
        line newSegment(points().at(index - 1).toPointF(), moveTo);
        move_junctions_to_new_segment(segment, newSegment);
    }

    point wirepoint = moveTo;
    wirepoint.set_is_junction(m_points[index].is_junction());
    m_points[index] = wirepoint;
}

/**
 * Is executed when the shape of the wire is about to change. This method can be
 * overridden by subclasses to prepare for such changes.
 */
void wire::about_to_change()
{

}

/**
 * Is executed when the shape of the wire has changed. This method can be
 * overridden by subclasses to react to such changes.
 */
void wire::has_changed()
{

}

void wire::set_point_is_junction(int index, bool isJunction)
{
    if (index < 0 || index > points_count() - 1) {
        return;
    }

    m_points[index].set_is_junction(isJunction);

    has_changed();
}

void wire::prepend_point(const QPointF& point)
{
    about_to_change();
    m_points.prepend(wire_system::point(point));
    has_changed();

    // Update junction
    if (points_count() >= 2) {
        set_point_is_junction(0, m_points.at(1).is_junction());
        set_point_is_junction(1, false);
    }

    m_manager->point_inserted(this, 0);
}

void wire::append_point(const QPointF& point)
{
    about_to_change();
    m_points.append(wire_system::point(point));
    has_changed();

    // Update junction
    if (points_count() > 2) {
        set_point_is_junction(points_count() - 1, m_points.at(points_count() - 2).is_junction());
        set_point_is_junction(points_count() - 2, false);
    }

    if (m_manager) {
        m_manager->point_inserted(this, points_count() - 1);
    }
}

void wire::move_line_segment_by(int index, const QVector2D& moveBy)
{
    // Do nothing if not moving
    if (moveBy.isNull()) {
        return;
    }

    // Have points_count()-2 in here because N points form N-1 line segments
    if (index < 0 || index > points_count() - 2) {
        return;
    }

    // Move connected junctions
    for (const auto& wire: m_connectedWires) {
        for (const auto& jIndex: wire->junctions()) {
            point point = wire->points().at(jIndex);
            line segment = line_segments().at(index);
            if (segment.contains_point(point.toPointF())) {
                // Don't move it if it is on one of the points
                if (segment.p1().toPoint() == point.toPoint() || segment.p2().toPoint() == point.toPoint()) {
                    continue;
                }
                wire->move_point_by(jIndex, moveBy);
            }
        }
    }

    // If this is the first or last segment we might need to add a new segment
    if (index == 0 || index == line_segments().count() - 1) {
        // Get the correct point
        point point;
        if (index == 0) {
            point = points().first();
        } else {
            point = points().last();
        }

        int pointIndex = (index == 0) ? 0 : points_count() - 1;

        // Check if the segment is connected to a node
        bool isConnected = m_manager && m_manager->point_is_attached(this, pointIndex);

        // Check if it's connected to a wire
        if (!isConnected && point.is_junction()) {
            isConnected = true;
        }

        // Add segment if it is connected
        if (isConnected) {
            add_segment(index);

            // Increment indices to account for inserted point
            if (index == 0) {
                index++;
            }
        }
    }

    // Move the line segment
    // Move point 1
    move_point_to(index, m_points[index] + moveBy.toPointF());
    // Move point 2
    move_point_to(index + 1, m_points[index + 1] + moveBy.toPointF());
}

void wire::add_segment(int index)
{
    if (index == 0) {
        // Add a point
        prepend_point(m_points.first().toPointF());
    } else {
        // Add a point
        append_point(m_points.last().toPointF());
    }
}

void wire::insert_point(int index, const QPointF& point)
{
    // Boundary check
    if (index < 0 || index >= points_count()) {
        return;
    }

    line segment = line_segments().at(index - 1);
    // If the point is not on the segment, move the junctions
    if (!segment.contains_point(point)) {
        // Find the closest point on the segment
        QPointF closestPoint = Utils::pointOnLineClosestToPoint(segment.p1(), segment.p2(), point);
        // Create two line that split the segment at the closest point
        line seg1(segment.p1(), closestPoint);
        line seg2(closestPoint, segment.p2());
        // Calculate what will be the new segments
        line seg1new(segment.p1(), point);
        line seg2new(point, segment.p2());
        // Move the junction on both lines
        move_junctions_to_new_segment(seg1, seg1new);
        move_junctions_to_new_segment(seg2, seg2new);
    }

    about_to_change();
    if (m_manager) {
        m_points.insert(index, wire_system::point(m_manager->settings().snapToGrid(point)));
    } else {
        m_points.insert(index, point);
    }
    has_changed();

    if (m_manager) {
        m_manager->point_inserted(this, index);
    }
}

void wire::move_point_by(int index, const QVector2D& moveBy)
{
    if (index < 0 || index > points_count() - 1) {
        return;
    }

    if (!m_manager) {
        move_point_to(index, points().at(index).toPointF() + moveBy.toPointF());
        return;
    }

    // If there are only two points (one line segment) and we are supposed to preserve
    // straight angles, we need to insert two additional points if we are not moving in
    // the direction of the line.
    if (points_count() == 2 && m_manager->settings().preserveStraightAngles) {
        const line line = line_segments().first();

        bool moveVertically = line.is_horizontal() && !qFuzzyIsNull(moveBy.y());
        bool moveHorizontally = line.is_vertical() && !qFuzzyIsNull(moveBy.x());
        // Only do this if we're not moving in the direction of the line. Because in that case
        // this is unnecessary as we're just moving one of the two points.
        if (!line.is_null() && (moveVertically || moveHorizontally)) {
            qreal lineLength = line.lenght();
            QPointF p;

            // The line is horizontal
            if (line.is_horizontal()) {
                QPointF leftPoint = line.p1();
                if (line.p2().x() < line.p1().x()) {
                    leftPoint = line.p2();
                }

                p.rx() = leftPoint.x() + static_cast<int>(lineLength/2);
                p.ry() = leftPoint.y();

                // The line is vertical
            } else {
                QPointF upperPoint = line.p1();
                if (line.p2().y() < line.p1().y()) {
                    upperPoint = line.p2();
                }

                p.rx() = upperPoint.x();
                p.ry() = upperPoint.y() + static_cast<int>(lineLength/2);
            }

            // Insert twice as these two points will form the new additional vertical or
            // horizontal line segment that is required to preserver straight angles.
            insert_point(1, p);
            insert_point(1, p);

            // Account for inserted points
            if (index == 1) {
                index += 2;
            }
        }
    }

    // Move the points
    QPointF currPoint = points().at(index).toPointF();
    // Preserve straight angles (if supposed to)
    if (m_manager->settings().preserveStraightAngles) {

        // Move previous point
        if (index >= 1) {
            QPointF prevPoint = points().at(index-1).toPointF();
            line line(prevPoint, currPoint);

            // Make sure that two wire points never collide
            if (points_count() > 3 && index >= 2 && wire_system::line(currPoint + moveBy.toPointF(), prevPoint).lenght() <= 2) {
                move_line_segment_by(index - 2, moveBy);
            }

            // Move junctions before the points are moved
            if (!line.is_null() && (line.is_horizontal() || line.is_vertical())) {
                // Move connected junctions
                for (const auto& wire: m_connectedWires) {
                    for (const auto& jIndex: wire->junctions()) {
                        const auto& point = wire->points().at(jIndex);
                        if (line.contains_point(point.toPointF())) {
                            // Don't move it if it is on one of the points
                            if (line.p1().toPoint() == point.toPoint() || line.p2().toPoint() == point.toPoint()) {
                                continue;
                            }
                            if (line.is_horizontal()) {
                                wire->move_point_by(jIndex, QVector2D(0, moveBy.y()));
                            } else {
                                wire->move_point_by(jIndex, QVector2D(moveBy.x(), 0));
                            }
                        }
                    }
                }
                // The line is horizontal
                if (line.is_horizontal()) {
                    move_point_to(index - 1, points().at(index - 1) + QPointF(0, moveBy.toPointF().y()));
                }
                    // The line is vertical
                else if (line.is_vertical()) {
                    move_point_to(index - 1, points().at(index - 1) + QPointF(moveBy.toPointF().x(), 0));
                }
            }
        }

        // Move next point
        if (index < points_count()-1) {
            QPointF nextPoint = points().at(index+1).toPointF();
            line line(currPoint, nextPoint);

            // Make sure that two wire points never collide
            if (points_count() > 3 && wire_system::line(currPoint + moveBy.toPointF(), nextPoint).lenght() <= 2) {
                move_line_segment_by(index + 1, moveBy);
            }

            // Move junctions before the points are moved
            if (!line.is_null() && (line.is_horizontal() || line.is_vertical())) {
                // Move connected junctions
                for (const auto& wire: m_connectedWires) {
                    for (const auto& jIndex: wire->junctions()) {
                        const auto& point = wire->points().at(jIndex);
                        if (line.contains_point(point.toPointF())) {
                            // Don't move it if it is on one of the points
                            if (line.p1().toPoint() == point.toPoint() || line.p2().toPoint() == point.toPoint()) {
                                continue;
                            }
                            if (line.is_horizontal()) {
                                wire->move_point_by(jIndex, QVector2D(0, moveBy.y()));
                            } else {
                                wire->move_point_by(jIndex, QVector2D(moveBy.x(), 0));
                            }
                        }
                    }
                }
                // The line is horizontal
                if (line.is_horizontal()) {
                    move_point_to(index + 1, points().at(index + 1) + QPointF(0, moveBy.toPointF().y()));
                }
                    // The line is vertical
                else if (line.is_vertical()) {
                    move_point_to(index + 1, points().at(index + 1) + QPointF(moveBy.toPointF().x(), 0));
                }
            }
        }
    }

    // Move the actual point itself
    move_point_to(index, currPoint + moveBy.toPointF());
}

bool wire::point_is_on_wire(const QPointF& point) const
{
    for (const line& lineSegment : line_segments()) {
        if (lineSegment.contains_point(point, 0)) {
            return true;
        }
    }

    return false;
}

void wire::move(const QVector2D& movedBy)
{
    // Ignore if it shouldn't move
    if (movedBy.isNull()) {
        return;
    }

    // Move junctions
    for (const auto& index : junctions()) {
        const auto& junction = points().at(index);
        for (const auto& wire : net()->wires()) {
            if (!wire->connected_wires().contains(this)) {
                continue;
            }
            if (wire->point_is_on_wire(junction.toPointF()) && !movedBy.isNull()) {
                move_point_by(index, -movedBy);
            }
        }
    }

    // Move junction on the wire
    for (const auto& wire : connected_wires()) {
        for (const auto& index : wire->junctions()) {
            const auto& point = wire->points().at(index);
            if (point_is_on_wire(point.toPointF())) {
                wire->move_point_by(index, movedBy);
            }
        }
    }

    // Move the points
    for (int index = 0; index < points_count(); index++) {
        move_point_to(index, m_points[index].toPointF() + movedBy.toPointF());
    }
}

void wire::remove_duplicate_points()
{
    int i = 0;
    while (i < points_count() - 1 && points_count() > 2) {
        point p1 = points().at(i);
        point p2 = points().at(i + 1);

        // Check if p2 is the same as p1
        if (p1 == p2) {
            // If p1 is not a junction itself then inherit from p2
            if (!p1.is_junction()) {
                set_point_is_junction(i, p2.is_junction());
            }
            if (m_manager) {
                m_manager->point_removed(this, i + 1);
            }
            m_points.removeAt(i + 1);
        } else {
            i++;
        }
    }
}

void wire::remove_obsolete_points()
{
    // Don't do anything if there are not at least three line segments
    if (points_count() < 3) {
        return;
    }

    // Compile a list of obsolete points
    auto it = m_points.begin() + 2;
    while (it != m_points.end()) {
        QPointF p1 = (*(it - 2)).toPointF();
        QPointF p2 = (*(it - 1)).toPointF();
        QPointF p3 = (*it).toPointF();

        // Check if p2 is on the line created by p1 and p3
        if (Utils::pointIsOnLine(QLineF(p1, p2), p3)) {
            if (m_manager) {
                m_manager->point_removed(this, m_points.indexOf(*(it - 1)));
            }
            it = m_points.erase(it - 1);
        }
        it++;
    }
}

void wire::simplify()
{
    about_to_change();
    remove_duplicate_points();
    remove_obsolete_points();
    has_changed();
}

bool wire::connect_wire(wire* wire)
{
    if (m_connectedWires.contains(wire)) {
        return false;
    }
    m_connectedWires.append(wire);
    return true;
}

void wire::setNet(const std::shared_ptr<wire_system::net>& net)
{
    m_net = net;
}

std::shared_ptr<wire_system::net> wire::net()
{
    return m_net;
}

void wire::disconnectWire(wire* wire)
{
    m_connectedWires.removeAll(wire);
}

manager* wire::manager()
{
    return m_manager;
}

void wire::remove_point(int index)
{
    about_to_change();
    // Move the junction on the previous and next segments
    if (index > 0 && index < points_count() - 1) {
        line newSegment(points().at(index - 1).toPointF(), points().at(index + 1).toPointF());
        move_junctions_to_new_segment(line_segments().at(index - 1), newSegment);
        move_junctions_to_new_segment(line_segments().at(index), newSegment);
    } else {
        for (const auto& wire: connected_wires()) {
            for (int junctionIndex: wire->junctions()) {
                QPointF point = wire->points().at(junctionIndex).toPointF();
                if (line_segments().first().contains_point(point)) {
                    wire->move_point_to(junctionIndex, points().at(1).toPointF());
                }
                if (line_segments().last().contains_point(point)) {
                    wire->move_point_to(junctionIndex, points().at(points_count() - 2).toPointF());
                }
            }
        }
    }
    m_points.remove(index);
    has_changed();
    if (m_manager) {
        m_manager->point_removed(this, index);
    }
}
