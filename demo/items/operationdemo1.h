#pragma once

#include "operation.h"

class OperationDemo1 : public Operation
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(OperationDemo1)

public:
    explicit OperationDemo1(QGraphicsItem* parent = nullptr);
    ~OperationDemo1() override = default;

    gpds::container to_container() const override;
    void from_container(const gpds::container& container) override;
    std::shared_ptr<QSchematic::Item> deepCopy() const override;

private:
    void copyAttributes(OperationDemo1& dest) const;
};
