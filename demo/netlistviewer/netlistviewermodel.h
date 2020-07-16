#pragma once

#include "qschematic/netlist.h"

#include "common/treemodel.h"

class Operation;
class OperationConnector;

class NetlistViewerModel : public TreeModel
{
public:
    explicit NetlistViewerModel( QObject* parent = nullptr );
    virtual ~NetlistViewerModel() override = default;

    void setNetlist( const QSchematic::Netlist<Operation*, OperationConnector*>& netlist );

private:
    Q_OBJECT
    Q_DISABLE_COPY( NetlistViewerModel )

    static QString pointerToString( const void* ptr );
};
