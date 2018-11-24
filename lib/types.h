#pragma once

namespace QSchematic {

    enum Direction {
        LeftToRight = 0,
        RightToLeft,
        TopToBottom,
        BottomToTop
    };

    enum ResizeHandle {
        ResizeTopLeft = 0,
        ResizeTop,
        ResizeTopRight,
        ResizeRight,
        ResizeBottomRight,
        ResizeBottom,
        ResizeBottomLeft,
        ResizeLeft
    };

}
