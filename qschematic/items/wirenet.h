#pragma once

#include <memory>
#include <QObject>
#include <QList>
#ifdef USE_GPDS
#include <gpds/serialize.hpp>
#endif

#include "wire_system/line.h"
#include "wire_system/net.h"
#include "qschematic_export.h"

namespace wire_system {
    class point;
}

using namespace wire_system;

namespace QSchematic {

    class Item;
    class Wire;
    class Label;
    class Scene;

    class QSCHEMATIC_EXPORT WireNet :
        public QObject,
#ifdef USE_GPDS
        public gpds::serialize,
#endif
        public wire_system::net
    {
        Q_OBJECT
        Q_DISABLE_COPY(WireNet)

    public:
        WireNet(QObject* parent = nullptr);
        virtual ~WireNet()  override;

#ifdef USE_GPDS
        virtual gpds::container to_container() const override;
        virtual void from_container(const gpds::container& container) override;
#endif

        bool addWire(const std::shared_ptr<wire>& wire) override;
        bool removeWire(const std::shared_ptr<wire> wire) override;
        void simplify();
        void set_name(const QString& name) override;
        void setHighlighted(bool highlighted);
        void setScene(Scene* scene);
        void updateLabelPos(bool updateParent = false) const;
        void wirePointMoved(Wire& wire, const point& point);

        QList<line> lineSegments() const;
        QList<QPointF> points() const;
        std::shared_ptr<Label> label();

    signals:
        void highlightChanged(bool highlighted);
        void contextMenuRequested(const QPoint& pos);

    private slots:
        void labelHighlightChanged(const Item& item, bool highlighted);
        void wireHighlightChanged(const Item& item, bool highlighted);
        void toggleLabel();

    private:
        QList<std::shared_ptr<WireNet>> nets() const;
        void highlight_global_net(bool highlighted);

        std::shared_ptr<Label> _label;
        Scene* _scene{};
    };

}
