#pragma once

#include <qschematic/items/widget.hpp>

namespace Items::Widgets
{

    class Textedit :
        public QSchematic::Items::Widget
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(Textedit)

    public:
        explicit
        Textedit(QGraphicsItem* parent = nullptr);

        ~Textedit() override = default;
    };

}
