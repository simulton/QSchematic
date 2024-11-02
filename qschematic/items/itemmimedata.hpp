#pragma once

#include <qschematic-export.h>

#include "item.hpp"

#include <QMimeData>

#include <memory>

namespace QSchematic::Items
{
    const QString MIME_TYPE_NODE = "qschematic/node";

    class QSCHEMATIC_EXPORT MimeData :
        public QMimeData
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(MimeData)

    public:
        explicit MimeData(std::shared_ptr<Item> item);
        ~MimeData() override = default;

        QStringList formats() const override;
        bool hasFormat(const QString& mimetype) const override;

        std::shared_ptr<Item> item() const;

    private:
        std::shared_ptr<Item> _item;
    };

}
