#pragma once

#include <QWidget>
#include "qschematic/netlist.h"

class NetlistViewerModel;
class NetlistViewerView;
class Operation;
class OperationConnector;

class NetlistViewer : public QWidget
{
public:
    NetlistViewer( QWidget* parent = nullptr );

    void setNetlist(const QSchematic::Netlist<Operation*, OperationConnector*>& netlist );

private:
    Q_OBJECT
    Q_DISABLE_COPY_MOVE( NetlistViewer )

    NetlistViewerModel* _model;
    NetlistViewerView* _view;
};
