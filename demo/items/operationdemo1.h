#pragma once

#include "operation.h"

class OperationDemo1 : public Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(OperationDemo1)

public:
    explicit OperationDemo1(QGraphicsItem* parent = nullptr);
    virtual ~OperationDemo1() override = default;

    virtual gpds::container to_container() const override;
    virtual void from_container(const gpds::container& container) override;
    virtual std::shared_ptr<QSchematic::Item> deepCopy() const override;

private:
    void copyAttributes(OperationDemo1& dest) const;
};
