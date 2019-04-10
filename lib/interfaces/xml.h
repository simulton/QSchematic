#pragma once

#include <QXmlStreamWriter>
#include <QXmlStreamReader>

namespace QSchematic {

    class Xml
    {
    public:
        virtual ~Xml() = default;

        virtual bool toXml(QXmlStreamWriter& xml) const = 0;
        virtual bool fromXml(QXmlStreamReader& xml) = 0;
    };

}
