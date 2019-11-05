#include "../qschematic/items/label.h"
#include "operationdemo1.h"
#include "operationconnector.h"
#include "itemtypes.h"

struct ConnectorAttribute {
    QPoint point;
    QString name;
};

OperationDemo1::OperationDemo1(QGraphicsItem* parent) :
    Operation(::ItemType::OperationDemo1Type, parent)
{
    setSize(160, 160);
    label()->setText(QStringLiteral("Demo 1"));

    QVector<ConnectorAttribute> connectorAttributes = {
        { QPoint(0, 2), QStringLiteral("in 1") },
        { QPoint(0, 4), QStringLiteral("in 2") },
        { QPoint(0, 6), QStringLiteral("in 3") },
        { QPoint(8, 4), QStringLiteral("out") }
    };

    for (const auto& c : connectorAttributes) {
        auto connector = QSchematic::make_origin<OperationConnector>(c.point, c.name);
        connector->label()->setVisible(true);
        addConnector(connector);
    }
}

Gpds::Container OperationDemo1::toContainer() const
{
    // Root
    Gpds::Container root;
    addItemTypeIdToContainer(root);
    root.addValue("operation", Operation::toContainer());

    return root;
}

void OperationDemo1::fromContainer(const Gpds::Container& container)
{
    // Root
    Operation::fromContainer( *container.getValue<Gpds::Container*>( "operation" ) );
}

QSchematic::OriginMgrT<QSchematic::Item> OperationDemo1::deepCopy() const
{
    auto clone = QSchematic::make_origin<OperationDemo1>(parentItem());
    copyAttributes(*(clone.get()));

    return clone;
}

void OperationDemo1::copyAttributes(OperationDemo1& dest) const
{
    Operation::copyAttributes(dest);
}
