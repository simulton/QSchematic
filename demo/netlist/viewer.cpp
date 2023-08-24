#include "viewer.hpp"
#include "model.hpp"
#include "view.hpp"

#include <QBoxLayout>

using namespace Netlist;

Viewer::Viewer(QWidget* parent) :
    QWidget( parent )
{
    // Model
    _model = new Model(this);

    // View
    _view = new View(this);
    _view->setModel(_model);

    // Main layout
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(_view);
    setLayout(layout);
}

void Viewer::setNetlist(const QSchematic::Netlist<Operation*, OperationConnector*>& netlist)
{
    Q_ASSERT(_model);
    _model->setNetlist(netlist);
}
