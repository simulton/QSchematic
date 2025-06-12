#pragma once

#include <qschematic/items/widget.hpp>

namespace Items::Widgets
{

    class Dial :
        public QSchematic::Items::Widget
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(Dial)

    public:
        explicit
        Dial(QGraphicsItem* parent = nullptr);

        ~Dial() override = default;
    };

}
