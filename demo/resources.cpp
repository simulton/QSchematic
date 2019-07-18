#include <QIcon>
#include "resources.h"

QIcon Resources::icon(IconType type)
{
    switch (type)
    {
    case ToggleGridIcon:
        return QIcon(QStringLiteral(":/icons/view-grid.png"));
    }

    return QIcon();
}
