#pragma once

class QIcon;

class Resources
{
public:
    enum IconType {
        ToggleGridIcon
    };

    static QIcon icon(IconType type);

private:
    Resources() = default;
    Resources(const Resources& other) = default;
    Resources(Resources&& other) = default;
    virtual ~Resources() = default;
};
