#pragma once

#include <QTreeView>

class NetlistViewerView : public QTreeView
{
public:
    explicit NetlistViewerView( QWidget* parent = nullptr );
    virtual ~NetlistViewerView() override = default;

private:
    Q_OBJECT
    Q_DISABLE_COPY_MOVE( NetlistViewerView )
};
