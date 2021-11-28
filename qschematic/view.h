#pragma once

#include <QGraphicsView>
#include "scene.h"
#include "qschematic_export.h"

namespace QSchematic {

    class QSCHEMATIC_EXPORT View :
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
        virtual ~View() override = default;

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
        virtual void keyPressEvent(QKeyEvent* event) override;
        virtual void wheelEvent(QWheelEvent* event) override;
        virtual void mouseMoveEvent(QMouseEvent* event) override;
        virtual void mousePressEvent(QMouseEvent* event) override;
        virtual void mouseReleaseEvent(QMouseEvent* event) override;

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
