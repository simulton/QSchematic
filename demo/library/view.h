#pragma once

#include <QTreeView>

namespace Library
{

    class View :
        public QTreeView
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(View)

    public:
        explicit View(QWidget* parent = nullptr);
        ~View() override = default;

        void setPixmapScale(qreal scale);

    private:
        void startDrag(Qt::DropActions supportedActions) override;

        qreal _scale;
    };

}
