#pragma once

#include <QWidget>

class Popup :
    public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Popup)

public:
    explicit Popup(QWidget* parent = nullptr);
    ~Popup() noexcept override = default;
};
