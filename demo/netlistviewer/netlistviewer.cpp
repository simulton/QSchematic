#include <QBoxLayout>
#include "netlistviewer.h"
#include "netlistviewermodel.h"
#include "netlistviewerview.h"

NetlistViewer::NetlistViewer( QWidget* parent ) :
    QWidget( parent )
{
    // Model
    _model = new NetlistViewerModel( this );

    // View
    _view = new NetlistViewerView( this );
    _view->setModel( _model );

    // Main layout
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget( _view );
    setLayout( layout );
}

void NetlistViewer::setNetlist( const QSchematic::Netlist<Operation*, OperationConnector*>& netlist )
{
    Q_ASSERT( _model );
    _model->setNetlist( netlist );
}
