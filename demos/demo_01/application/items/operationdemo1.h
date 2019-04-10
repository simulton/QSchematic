#pragma once

#include "operation.h"

class OperationDemo1 : public Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(OperationDemo1)

public:
    explicit OperationDemo1(QGraphicsItem* parent = nullptr);
    virtual ~OperationDemo1() override = default;

    virtual bool toXml(QXmlStreamWriter& xml) const override;
    virtual bool fromXml(QXmlStreamReader& reader) override;
    virtual std::unique_ptr<QSchematic::Item> deepCopy() const override;

private:
    void copyAttributes(OperationDemo1& dest) const;
};
