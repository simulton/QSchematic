#include "textedit.hpp"
#include "../itemtypes.hpp"

#include <QTextEdit>

using namespace Items::Widgets;

Textedit::Textedit(QGraphicsItem* parent) :
    QSchematic::Items::Widget(::ItemType::WidgetTextedit, parent)
{
    setWidget([]{ return new QTextEdit; });
}
