#include <QPainter>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include <QInputDialog>
#include <QGraphicsDropShadowEffect>

#include "qschematic/scene.h"
#include "qschematic/items/label.h"
#include "qschematic/commands/commanditemremove.h"
#include "qschematic/commands/commanditemvisibility.h"
#include "qschematic/commands/commandlabelrename.h"

#include "operation.h"
#include "operationconnector.h"
#include "commands/commandnodeaddconnector.h"
#include "popup/popup_operation.hpp"

const QColor COLOR_BODY_FILL   = QColor( QStringLiteral( "#e0e0e0" ) );
const QColor COLOR_BODY_BORDER = QColor(Qt::black);
const QColor SHADOW_COLOR      = QColor(63, 63, 63, 100);
const qreal PEN_WIDTH          = 1.5;
const qreal SHADOW_OFFSET      = 7;
const qreal SHADOW_BLUR_RADIUS = 10;

Operation::Operation(int type, QGraphicsItem* parent) :
    QSchematic::Node(type, parent)
{
    // Label
    _label = std::make_shared<QSchematic::Label>();
    _label->setParentItem(this);
    _label->setVisible(true);
    _label->setMovable(true);
    _label->setPos(0, 120);
    _label->setText(QStringLiteral("Generic"));
    connect(this, &QSchematic::Node::sizeChanged, [this]{
        label()->setConnectionPoint(sizeRect().center());
    });
    connect(this, &QSchematic::Item::settingsChanged, [this]{
        label()->setConnectionPoint(sizeRect().center());
        label()->setSettings(_settings);
    });

    // Misc
    setSize(160, 80);
    setAllowMouseResize(true);
    setAllowMouseRotate(true);
    setConnectorsMovable(true);
    setConnectorsSnapPolicy(QSchematic::Connector::NodeSizerectOutline);
    setConnectorsSnapToGrid(true);

    // Drop shadow
    auto graphicsEffect = new QGraphicsDropShadowEffect(this);
    graphicsEffect->setOffset(SHADOW_OFFSET);
    graphicsEffect->setBlurRadius(SHADOW_BLUR_RADIUS);
    graphicsEffect->setColor(SHADOW_COLOR);
    setGraphicsEffect(graphicsEffect);
}

Operation::~Operation()
{
    dissociate_item(_label);
}

gpds::container Operation::to_container() const
{
    // Root
    gpds::container root;
    addItemTypeIdToContainer(root);
    root.add_value("node", QSchematic::Node::to_container());
    root.add_value("label", _label->to_container());

    return root;
}

void Operation::from_container(const gpds::container& container)
{
    // Root
    QSchematic::Node::from_container(*container.get_value<gpds::container*>("node").value());
    _label->from_container(*container.get_value<gpds::container*>("label").value());
}

std::unique_ptr<QWidget> Operation::popup() const
{
    return std::make_unique<PopupOperation>(*this);
}

std::shared_ptr<QSchematic::Item> Operation::deepCopy() const
{
    auto clone = std::make_shared<Operation>(::ItemType::OperationType, parentItem());
    copyAttributes(*clone);

    return clone;
}

void Operation::copyAttributes(Operation& dest) const
{
    QSchematic::Node::copyAttributes(dest);

    // Label
    dest._label = std::dynamic_pointer_cast<QSchematic::Label>(_label->deepCopy());
    dest._label->setParentItem(&dest);
}

std::shared_ptr<QSchematic::Label> Operation::label() const
{
    return _label;
}


void Operation::setText(const QString& text)
{
    Q_ASSERT(_label);

    _label->setText(text);
}

QString Operation::text() const
{
    Q_ASSERT(_label);

    return _label->text();
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
            painter->drawRoundedRect(sizeRect(), radius, radius);
        }
    }

    // Resize handles
    if (isSelected() && allowMouseResize()) {
        paintResizeHandles(*painter);
    }

    // Rotate handle
    if (isSelected() && allowMouseRotate()) {
        paintRotateHandle(*painter);
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
            if (!scene())
                return;

            const QString& newText = QInputDialog::getText(nullptr, "Rename Connector", "New connector text", QLineEdit::Normal, label()->text());

            scene()->undoStack()->push(new QSchematic::CommandLabelRename(label().get(), newText));
        });

        // Label visibility
        QAction* labelVisibility = new QAction;
        labelVisibility->setCheckable(true);
        labelVisibility->setChecked(label()->isVisible());
        labelVisibility->setText("Label visible");
        connect(labelVisibility, &QAction::toggled, [this](bool enabled) {
            if (!scene())
                return;

            scene()->undoStack()->push(new QSchematic::CommandItemVisibility(label(), enabled));
        });

        // Align label
        QAction* alignConnectorLabels = new QAction;
        alignConnectorLabels->setText("Align connector labels");
        connect(alignConnectorLabels, &QAction::triggered, [this] {
            this->alignConnectorLabels();
        });

        // Add connector
        QAction* newConnector = new QAction;
        newConnector->setText("Add connector");
        connect(newConnector, &QAction::triggered, [this, event] {
            if (!scene())
                return;

            auto connector = std::make_shared<OperationConnector>(event->pos().toPoint(), QStringLiteral("Unnamed"), this);

            scene()->undoStack()->push(new Commands::CommandNodeAddConnector(this, connector));
        });

        // Duplicate
        QAction* duplicate = new QAction;
        duplicate->setText("Duplicate");
        connect(duplicate, &QAction::triggered, [this] {
            if (!scene())
                return;

            auto clone = deepCopy();
            clone->setPos( pos() + QPointF(5*_settings.gridSize, 5*_settings.gridSize));
            scene()->addItem(std::move(clone));
        });

        // Delete
        QAction* deleteFromModel = new QAction;
        deleteFromModel->setText("Delete");
        connect(deleteFromModel, &QAction::triggered, [this] {
            if (!scene())
                return;

            // Retrieve smart pointer
            std::shared_ptr<QSchematic::Item> itemPointer;
            for (auto& i : scene()->items()) {
                if (i.get() == this) {
                    itemPointer = i;
                    break;
                }
            }
            if (!itemPointer) {
                return;
            }

            // Issue command
            scene()->undoStack()->push(new QSchematic::CommandItemRemove(scene(), itemPointer));
        });

        // Assemble
        menu.addAction(text);
        menu.addAction(labelVisibility);
        menu.addSeparator();
        menu.addAction(newConnector);
        menu.addAction(alignConnectorLabels);
        menu.addSeparator();
        menu.addAction(duplicate);
        menu.addAction(deleteFromModel);
    }

    // Sow the menu
    menu.exec(event->screenPos());
}
