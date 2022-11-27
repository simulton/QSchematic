#include "model.h"
#include "../common/treeitem.h"
#include "../items/operation.h"
#include "../items/operationconnector.h"

using namespace Netlist;

Model::Model(QObject* parent) :
    TreeModel( parent )
{
    // Set header
    _rootItem->setData( { "Name", "Address" } );
}

void Model::setNetlist(const QSchematic::Netlist<Operation*, OperationConnector*>& netlist)
{
    // Clear previous content
    clear();

    // Make sure that there are nets available
    if ( netlist.nets.empty() ) {
        return;
    }

    // Create three model
    {
        Q_ASSERT( _rootItem );
        beginInsertRows( QModelIndex(), 0, static_cast<int>( netlist.nets.size() )-1 );

        for ( const auto& net : netlist.nets ) {

            // Net
            TreeItem* netItem = new TreeItem( { net.name, pointerToString( &net ) } );
            _rootItem->appendChild( netItem );

            // Nodes
            TreeItem* nodesItem = new TreeItem( { QStringLiteral( "Nodes" ), "" } );
            netItem->appendChild( nodesItem );
            for ( const auto& node : net.nodes ) {
                Q_ASSERT( node );
                TreeItem* nodeItem = new TreeItem( { node->text(), pointerToString( &node ) } );
                nodesItem->appendChild( nodeItem );
            }

            // Connectors
            TreeItem* connectorsItem = new TreeItem( { QStringLiteral( "Connectors" ), "" } );
            netItem->appendChild( connectorsItem );
            for ( const auto& connector : net.connectors ) {
                Q_ASSERT( connector );
                Q_ASSERT( connector->label() );
                TreeItem* connectorItem = new TreeItem( { connector->label()->text(), pointerToString( connector ) } );
                connectorsItem->appendChild( connectorItem );
            }

            // Wires
            TreeItem* wiresItem = new TreeItem( { QStringLiteral( "Wires" ), "" } );
            netItem->appendChild( wiresItem );
            for ( const auto& wire : net.wires ) {
                Q_ASSERT( wire );
                TreeItem* wireItem = new TreeItem( { "Wire", pointerToString( wire ) } );
                wiresItem->appendChild( wireItem );
            }
        }

        endInsertRows();
    }
}

QString Model::pointerToString(const void* ptr)
{
    return QString( "0x%1" ).arg( (quintptr)ptr, QT_POINTER_SIZE * 2, 16, QChar('0') );
}
