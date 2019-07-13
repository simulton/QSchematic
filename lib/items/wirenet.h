#pragma once

#include <memory>
#include <QObject>
#include <QList>
#include "serialize.h"
#include "line.h"

namespace QSchematic {

    class Item;
    class Wire;
    class WirePoint;
    class Label;

    class WireNet : public QObject, public Gpds::Serialize
    {
        Q_OBJECT
        Q_DISABLE_COPY(WireNet)

    public:
        WireNet(QObject* parent = nullptr);

        virtual Gpds::Container toContainer() const override;
        virtual void fromContainer(const Gpds::Container& container) override;

        bool addWire(const std::shared_ptr<Wire>& wire);
        bool removeWire(const std::shared_ptr<Wire>& wire);
        bool contains(const std::shared_ptr<Wire>& wire) const;
        void simplify();
        void setName(const QString& name);
        void setHighlighted(bool highlighted);

        QString name() const;
        QList<std::shared_ptr<Wire>> wires() const;
        QList<Line> lineSegments() const;
        QList<QPointF> points() const;
        std::shared_ptr<Label> label();

    signals:
        void pointMoved(Wire& wire, WirePoint& point);
        void highlightChanged(bool highlighted);

    private slots:
        void wirePointMoved(Wire& wire, WirePoint& point);
        void labelHighlightChanged(const Item& item, bool highlighted);
        void wireHighlightChanged(const Item& item, bool highlighted);

    private:
        void updateWireJunctions();

        QList<std::shared_ptr<Wire>> _wires;
        QString _name;
        std::shared_ptr<Label> _label;
    };

}
