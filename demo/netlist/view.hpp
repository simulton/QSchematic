#pragma once

#include <QTreeView>

namespace Netlist
{

    class View :
        public QTreeView
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(View)

    public:
        explicit View(QWidget* parent = nullptr);
        ~View() override = default;
    };

}
