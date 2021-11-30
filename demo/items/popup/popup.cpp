#include "popup.hpp"

Popup::Popup(QWidget* parent) :
    QWidget(parent)
{
    // Prevent widget from receiving focus
    setFocusPolicy(Qt::NoFocus);

    // Adjust colors
    QPalette p = palette();
    p.setColor(QPalette::Window, QColor("#F2F281"));
    setPalette(p);
}
