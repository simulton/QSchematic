#pragma once

#include <QObject>
#include <QList>
#include <QSharedPointer>
#include <memory>
#include "../interfaces/json.h"
#include "line.h"

namespace QSchematic {

    class Item;
    class Wire;
    class WirePoint;
    class Label;

    class WireNet : public QObject, public Json
    {
        Q_OBJECT
        Q_DISABLE_COPY(WireNet)

    public:
        WireNet(QObject* parent = nullptr);
        virtual ~WireNet() override;

        virtual QJsonObject toJson() const override;
        virtual bool fromJson(const QJsonObject& object) override;

        bool addWire(Wire& wire);
        bool removeWire(Wire& wire);
        bool contains(const Wire& wire) const;
        void simplify();
        void setName(const QString& name);
        void setHighlighted(bool highlighted);

        QString name() const;
        QList<Wire*> wires() const;
        QList<Line> lineSegments() const;
        QList<QPoint> points() const;
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

        QList<Wire*> _wires;
        QString _name;
        std::shared_ptr<Label> _label;
    };

}
