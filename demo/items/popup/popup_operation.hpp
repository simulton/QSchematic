#pragma once

#include "popup.hpp"
#include "../operation.h"

#include <qschematic/items/label.h>

#include <QFormLayout>
#include <QLabel>

class PopupOperation :
    public Popup
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(PopupOperation)

public:
    explicit
    PopupOperation(const Operation& op)
    {
        // Main layout
        auto layout = new QFormLayout;
        layout->addRow("Type:", new QLabel("Operation"));
        layout->addRow("Name:", new QLabel(op.label()->text()));
        setLayout(layout);
    }

    ~PopupOperation() noexcept override = default;
};
