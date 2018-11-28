#pragma once

class QJsonObject;

namespace QSchematic {

    class Json
    {
    public:
        virtual ~Json() = default;

        virtual QJsonObject toJson() const = 0;
        virtual bool fromJson(const QJsonObject& object) = 0;
    };

}
