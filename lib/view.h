#pragma once

#include <QGraphicsView>
#include "scene.h"

namespace QSchematic {

    class View : public QGraphicsView
    {
        Q_OBJECT
        Q_DISABLE_COPY(View)

    public:
        explicit View(QWidget* parent = nullptr);
        virtual ~View() override = default;

        void setSettings(const Settings& settings);
        qreal zoomValue() const;

    signals:
        void zoomChanged(qreal factor);

    public slots:
        void setMode(Scene::Mode mode);
        void setZoomValue(qreal factor);

    protected:
        virtual void keyPressEvent(QKeyEvent* event) override;
        virtual void wheelEvent(QWheelEvent* event) override;
        virtual void mouseMoveEvent(QMouseEvent* event) override;
        virtual void mousePressEvent(QMouseEvent* event) override;
        virtual void mouseReleaseEvent(QMouseEvent* event) override;

    private:
        void updateScale();

        Settings _settings;
        qreal _scaleFactor;
        bool _pan;
        int _panStartX, _panStartY;
    };

}
