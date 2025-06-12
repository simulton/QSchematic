#include "dial.hpp"
#include "../itemtypes.hpp"

#include <QDial>

using namespace Items::Widgets;

Dial::Dial(QGraphicsItem* parent) :
    QSchematic::Items::Widget(::ItemType::WidgetDial, parent)
{
    setWidget([]{ return new QDial; });
}
