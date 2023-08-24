#pragma once

#include "popup.hpp"
#include "../operationconnector.hpp"

#include <qschematic/items/label.hpp>

#include <QFormLayout>
#include <QLabel>

class PopupConnector :
    public Popup
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(PopupConnector)

public:
    explicit
    PopupConnector(const OperationConnector& conn)
    {
        // Main layout
        auto layout = new QFormLayout;
        layout->addRow("Type:", new QLabel("Connector"));
        layout->addRow("Name:", new QLabel(conn.label()->text()));
        setLayout(layout);
    }

    ~PopupConnector() noexcept override = default;
};
