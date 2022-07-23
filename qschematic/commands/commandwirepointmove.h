#pragma once

#include "commandbase.h"
#include "../items/wire.h"

#include <QVector>
#include <QVector2D>

#include <memory>

class QVector2D;

namespace QSchematic
{
    class Item;

    class CommandWirepointMove :
        public UndoCommand
    {
    public:
        CommandWirepointMove(Scene* scene, const std::shared_ptr<Wire>& wire, int index,
                             const QPointF& pos, QUndoCommand* parent = nullptr);

        int id() const override;
        bool mergeWith(const QUndoCommand* command) override;
        void undo() override;
        void redo() override;

    private:
        std::shared_ptr<Wire> _wire;
        QVector<QPointF> _oldPos;
        QVector<QPointF> _newPos;
        std::shared_ptr<net> _oldNet;
        std::shared_ptr<net> _newNet;
        Scene* _scene;
    };

}
