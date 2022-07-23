#pragma once

#include "scene.h"

#include <QGraphicsView>

namespace QSchematic {

    class View :
        public QGraphicsView
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(View)

    public:
        enum Mode {
            NormalMode,
            PanMode
        };

        explicit View(QWidget* parent = nullptr);
        ~View() override = default;

        void setScene(Scene* scene);
        void setSettings(const Settings& settings);
        qreal zoomValue() const;

    signals:
        void zoomChanged(qreal factor);
        void modeChanged(Mode newMode);

    public slots:
        void setZoomValue(qreal factor);
        void fitInView();

    protected:
        void keyPressEvent(QKeyEvent* event) override;
        void wheelEvent(QWheelEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void mousePressEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;

    private:
        void updateScale();
        void setMode(Mode newMode);

        Scene* _scene;
        Settings _settings;
        qreal _scaleFactor;
        Mode _mode;
        QPoint _panStart;
    };
}
