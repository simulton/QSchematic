#include "item.h"

#include <QMimeData>

#include <memory>

namespace QSchematic
{
    const QString MIME_TYPE_NODE = "qschematic/node";

    class ItemMimeData :
        public QMimeData
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(ItemMimeData)

    public:
        explicit ItemMimeData(std::shared_ptr<Item> item);
        ~ItemMimeData() override = default;

        QStringList formats() const override;
        bool hasFormat(const QString& mimetype) const override;

        std::shared_ptr<Item> item() const;

    private:
        std::shared_ptr<Item> _item;
    };

}
