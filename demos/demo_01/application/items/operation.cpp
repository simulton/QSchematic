#include <QPainter>
#include <QJsonObject>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include <QInputDialog>
#include <QGraphicsDropShadowEffect>
#include "../../../lib/scene.h"
#include "../../../lib/items/label.h"
#include "../../../lib/commands/commanditemremove.h"
#include "../../../lib/commands/commanditemvisibility.h"
#include "../../../lib/commands/commandlabelrename.h"
#include "operation.h"
#include "operationconnector.h"
#include "../commands/commandnodeaddconnector.h"

const QColor COLOR_BODY_FILL   = QColor(Qt::gray).lighter(140);
const QColor COLOR_BODY_BORDER = QColor(Qt::black);
const QColor COLOR_SHADOW      = QColor(150, 150, 150, 50);
const qreal PEN_WIDTH          = 1.5;
const qreal SHADOW_OFFSET      = 7;
const qreal SHADOW_BLUR_RADIUS = 10;
#include <QGraphicsBlurEffect>
Operation::Operation(int type, QGraphicsItem* parent) :
    QSchematic::Node(type, parent)
{
    // Misc
    label()->setText(QStringLiteral("Generic"));
    label()->setMovable(true);
    label()->setVisible(true);
    connect(this, &QSchematic::Node::sizeChanged, [this]{
        label()->setConnectionPoint(sizeSceneRect().center());
        repositionLabel();
    });
    connect(this, &QSchematic::Item::settingsChanged, [this]{
        label()->setConnectionPoint(sizeSceneRect().center());
        repositionLabel();
    });
    connect(label().get(), &QSchematic::Label::textChanged, this, &Operation::repositionLabel);
    setSize(8, 4);
    setAllowMouseResize(true);
    setConnectorsMovable(true);
    setConnectorsSnapPolicy(QSchematic::Connector::NodeSizerectOutline);
    setConnectorsSnapToGrid(true);

    // Drop shadow
    auto graphicsEffect = new QGraphicsDropShadowEffect(this);
    graphicsEffect->setOffset(SHADOW_OFFSET);
    graphicsEffect->setBlurRadius(SHADOW_BLUR_RADIUS);
    graphicsEffect->setColor(QColor(63, 63, 63, 100));
    setGraphicsEffect(graphicsEffect);
}

QJsonObject Operation::toJson() const
{
    QJsonObject object;

    object.insert("node", QSchematic::Node::toJson());
    addTypeIdentifierToJson(object);

    return object;
}

bool Operation::fromJson(const QJsonObject& object)
{
    QSchematic::Node::fromJson(object["node"].toObject());

    return true;
}

std::unique_ptr<QSchematic::Item> Operation::deepCopy() const
{
    auto clone = std::make_unique<Operation>(::ItemType::OperationType, parentItem());
    copyAttributes(*(clone.get()));

    return clone;
}

void Operation::copyAttributes(Operation& dest) const
{
    QSchematic::Node::copyAttributes(dest);
}

void Operation::repositionLabel()
{
    const auto& centerDiff = sizeSceneRect().center() - label()->textRect().center();
    label()->setPos(centerDiff);
}

QRectF Operation::boundingRect() const
{
    qreal adj = SHADOW_BLUR_RADIUS + 1.5;

    return QRectF(sizeSceneRect()).adjusted(-adj, -adj, adj, adj);
}

void Operation::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    // Draw the bounding rect if debug mode is enabled
    if (_settings.debug) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(Qt::red));
        painter->drawRect(boundingRect());
    }

    // Body
    {
        // Common stuff
        QRect bodyRect(0, 0, size().width()*_settings.gridSize, size().height()*_settings.gridSize);
        qreal radius = _settings.gridSize/2;

        // Body
        {
            // Pen
            QPen pen;
            pen.setWidthF(PEN_WIDTH);
            pen.setStyle(Qt::SolidLine);
            pen.setColor(COLOR_BODY_BORDER);

            // Brush
            QBrush brush;
            brush.setStyle(Qt::SolidPattern);
            brush.setColor(COLOR_BODY_FILL);

            // Draw the component body
            painter->setPen(pen);
            painter->setBrush(brush);
            painter->drawRoundedRect(bodyRect, radius, radius);
        }
    }

    // Resize handles
    if (isSelected() and allowMouseResize()) {
        paintResizeHandles(*painter);
    }
}

void Operation::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    // Create the menu
    QMenu menu;
    {
        // Text
        QAction* text = new QAction;
        text->setText("Rename ...");
        connect(text, &QAction::triggered, [this] {
            const QString& newText = QInputDialog::getText(nullptr, "Rename Connector", "New connector text", QLineEdit::Normal, label()->text());

            if (scene()) {
                scene()->undoStack()->push(new QSchematic::CommandLabelRename(label(), newText));
            } else {
                label()->setText(newText);
            }
        });

        // Label visibility
        QAction* labelVisibility = new QAction;
        labelVisibility->setCheckable(true);
        labelVisibility->setChecked(label()->isVisible());
        labelVisibility->setText("Label visible");
        connect(labelVisibility, &QAction::toggled, [this](bool enabled) {
            if (scene()) {
                scene()->undoStack()->push(new QSchematic::CommandItemVisibility(label(), enabled));
            } else {
                label()->setVisible(enabled);
            }
        });

        // Add connector
        QAction* newConnector = new QAction;
        newConnector->setText("Add connector");
        connect(newConnector, &QAction::triggered, [this, event] {
            auto connector = std::make_shared<OperationConnector>(event->pos().toPoint(), QStringLiteral("Unnamed"), this);

            if (scene()) {
                scene()->undoStack()->push(new CommandNodeAddConnector(this, connector));
            } else {
                addConnector(connector);
            }
        });

        // Delete
        QAction* deleteFromModel = new QAction;
        deleteFromModel->setText("Delete");
        connect(deleteFromModel, &QAction::triggered, [this] {
            if (scene()) {
                scene()->undoStack()->push(new QSchematic::CommandItemRemove(scene(), this));
            }
        });

        // Assemble
        menu.addAction(text);
        menu.addAction(labelVisibility);
        menu.addSeparator();
        menu.addAction(newConnector);
        menu.addSeparator();
        menu.addAction(deleteFromModel);
    }

    // Sow the menu
    menu.exec(event->screenPos());
}
