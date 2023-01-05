#pragma once

#include "scene.h"

#include <QGraphicsView>

namespace QSchematic
{

    /**
     * A QWidget to display & interact with a QSchematic scene.
     */
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

        qreal zoom_factor_min  = 0.25;
        qreal zoom_factor_max  = 10.0;
        qreal zoom_factor_step = 0.10;
        qreal fitall_padding   = 20.0;

        /**
         * Constructor.
         *
         * @param parent The parent widget.
         */
        explicit
        View(QWidget* parent = nullptr);

        /**
         * Destructor.
         */
        ~View() override = default;

        /**
         * Set the scene.
         *
         * @param scene The scene.
         */
        void
        setScene(Scene* scene);

        /**
         * Set settings.
         *
         * @param settings The settings.
         */
        void
        setSettings(const Settings& settings);

        /**
         * Get current zoom value.
         *
         * @return Current zoom value.
         */
        [[nodiscard]]
        qreal
        zoomValue() const;

    signals:
        void zoomChanged(qreal factor);
        void modeChanged(Mode newMode);

    public slots:
        /**
         * Set the zoom value.
         *
         * @param factor The zoom value.
         */
        void
        setZoomValue(qreal factor);

        /**
         * Fit everything into the view port.
         *
         * @details This modifies the view transformation to ensure that all items in the scene are visible in the
         *          viewport.
         */
        void
        fitInView();

    protected:
        // QGraphicsView overrides
        void keyPressEvent(QKeyEvent* event) override;
        void wheelEvent(QWheelEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void mousePressEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;

    private:
        void
        updateScale();

        void
        setMode(Mode newMode);

        Scene* _scene = nullptr;
        Settings _settings;
        qreal _scaleFactor = 1.0;
        Mode _mode = Mode::NormalMode;
        QPoint _panStart;
    };
}
