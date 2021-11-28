#pragma once

#include <qschematic/netlist.h>

#include "common/treemodel.h"

class Operation;
class OperationConnector;

namespace Netlist
{

    class Model :
        public TreeModel
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(Model)

    public:
        explicit Model(QObject* parent = nullptr);
        ~Model() override = default;

        void setNetlist(const QSchematic::Netlist<Operation*, OperationConnector*>& netlist);

    private:
        [[nodiscard]]
        QString pointerToString(const void* ptr);
    };

}
