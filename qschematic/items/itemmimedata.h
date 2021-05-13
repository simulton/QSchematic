#include <memory>
#include <QMimeData>
#include "item.h"
#include "qschematic_export.h"

namespace QSchematic
{
    const QString MIME_TYPE_NODE = "qschematic/node";

    class QSCHEMATIC_EXPORT ItemMimeData :
        public QMimeData
    {
        Q_OBJECT
        Q_DISABLE_COPY(ItemMimeData)

    public:
        explicit ItemMimeData(std::shared_ptr<Item> item);
        virtual ~ItemMimeData() override = default;

        virtual QStringList formats() const override;
        virtual bool hasFormat(const QString& mimetype) const override;

        std::shared_ptr<Item> item() const;

    private:
        std::shared_ptr<Item> _item;
    };

}
