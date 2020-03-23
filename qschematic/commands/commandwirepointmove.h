#pragma once

#include <memory>
#include <QVector>
#include <QVector2D>
#include "../items/wire.h"
#include "commandbase.h"

class QVector2D;

namespace QSchematic
{
    class Item;

    class CommandWirepointMove : public UndoCommand
    {
    public:
        CommandWirepointMove(Scene* scene, const std::shared_ptr<Wire>& wire, int index,
                             const QPointF& pos, QUndoCommand* parent = nullptr);

        virtual int id() const override;
        virtual bool mergeWith(const QUndoCommand* command) override;
        virtual void undo() override;
        virtual void redo() override;

    private:
        std::shared_ptr<Wire> _wire;
        QVector<QPointF> _oldPos;
        QVector<QPointF> _newPos;
        std::shared_ptr<net> _oldNet;
        std::shared_ptr<net> _newNet;
        Scene* _scene;
    };

}
