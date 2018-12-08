#pragma once

#include <QTreeView>

class ItemsLibraryView : public QTreeView
{
    Q_OBJECT
    Q_DISABLE_COPY(ItemsLibraryView)

public:
    explicit ItemsLibraryView(QWidget* parent = nullptr);
    virtual ~ItemsLibraryView() override = default;

    void setPixmapScale(qreal scale);

private:
    virtual void startDrag(Qt::DropActions supportedActions) override;

    qreal _scale;
};
