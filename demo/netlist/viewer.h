#pragma once

#include <qschematic/netlist.h>

#include <QWidget>

class Operation;
class OperationConnector;

namespace Netlist
{
    class Model;
    class View;

    class Viewer :
        public QWidget
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(Viewer)

    public:
        Viewer(QWidget *parent = nullptr);
        ~Viewer() override = default;

        void setNetlist(const QSchematic::Netlist<Operation*, OperationConnector*>& netlist);

    private:
        Model* _model = nullptr;
        View* _view = nullptr;
    };

}
