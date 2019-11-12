#pragma once

#include <memory>
#include <QObject>
#include <QList>
#include <gpds/serialize.h>
#include "line.h"

namespace QSchematic {

    class Item;
    class Wire;
    class WirePoint;
    class Label;

    class WireNet : public QObject, public gpds::serialize, public std::enable_shared_from_this<WireNet>
    {
        Q_OBJECT
        Q_DISABLE_COPY(WireNet)

    public:
        WireNet(QObject* parent = nullptr);

        virtual gpds::container to_container() const override;
        virtual void from_container(const gpds::container& container) override;

        bool addWire(const std::shared_ptr<Wire>& wire);
        bool removeWire(const std::shared_ptr<Wire>& wire);
        bool contains(const std::shared_ptr<Wire>& wire) const;
        void simplify();
        void setName(const std::string& name);
        void setName(const QString& name);
        void setHighlighted(bool highlighted);

        QString name() const;
        QList<std::shared_ptr<Wire>> wires() const;
        QList<Line> lineSegments() const;
        QList<QPointF> points() const;
        std::shared_ptr<Label> label();

    signals:
        void pointMoved(Wire& wire, WirePoint& point);
        void pointMovedByUser(Wire& wire, int index);
        void highlightChanged(bool highlighted);
        void contextMenuRequested(const QPoint& pos);

    private slots:
        void wirePointMoved(Wire& wire, WirePoint& point);
        void wirePointMovedByUser(Wire& wire, int index);
        void labelHighlightChanged(const Item& item, bool highlighted);
        void wireHighlightChanged(const Item& item, bool highlighted);
        void toggleLabel();

    private:

        QList<std::shared_ptr<Wire>> _wires;
        QString _name;
        std::shared_ptr<Label> _label;
        void updateLabelPos() const;
    };

}
