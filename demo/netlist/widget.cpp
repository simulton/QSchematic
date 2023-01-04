#include "widget.h"
#include "viewer.h"
#include "../items/operation.h"
#include "../items/operationconnector.h"

#include <qschematic/netlist_writer_json.h>

#include <QJsonDocument>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QTabWidget>

using namespace Netlist;

Widget::Widget(QWidget* parent) :
    QWidget(parent)
{
    // Memory viewer
    m_memory_viewer = new Viewer;

    // JSON viewer
    m_json_viewer = new QPlainTextEdit;
    m_json_viewer->setReadOnly(true);

    // Tab
    auto tab_widget = new QTabWidget;
    tab_widget->addTab(m_memory_viewer, tr("Memory"));
    tab_widget->addTab(m_json_viewer, tr("JSON"));

    // Layout
    auto layout = new QVBoxLayout;
    layout->addWidget(tab_widget);
    setLayout(layout);
}

void
Widget::setNetlist(const QSchematic::Netlist<Operation*, OperationConnector*>& netlist)
{
    // Memory
    m_memory_viewer->setNetlist(netlist);

    // JSON
    auto json = QSchematic::toJson(netlist);
    m_json_viewer->setPlainText(QJsonDocument(json).toJson(QJsonDocument::Indented));
}
