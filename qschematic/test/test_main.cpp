#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

#include <QApplication>

int
main(int argc, char** argv)
{
    // The scene is a QGraphicsScene, so these tests need a QApplication. Nothing is ever shown,
    // so there is no reason to load a platform plugin that needs a display.
    qputenv("QT_QPA_PLATFORM", "offscreen");

    QApplication app(argc, argv);

    doctest::Context context;
    context.applyCommandLine(argc, argv);

    return context.run();
}
