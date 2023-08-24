#pragma once

#include <qschematic/netlist.hpp>

#include <QWidget>

class Operation;
class OperationConnector;

class QPlainTextEdit;

namespace Netlist
{
    class Viewer;

    class Widget :
        public QWidget
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(Widget)

    public:
        explicit
        Widget(QWidget* parent = nullptr);

        ~Widget() override = default;

        void
        setNetlist(const QSchematic::Netlist<Operation*, OperationConnector*>& netlist);

    private:
        Viewer* m_memory_viewer = nullptr;
        QPlainTextEdit* m_json_viewer = nullptr;
    };

}