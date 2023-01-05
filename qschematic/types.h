#pragma once

namespace QSchematic
{

    enum Direction {
        LeftToRight = 0,
        RightToLeft,
        TopToBottom,
        BottomToTop
    };

    enum RectanglePoint {
        RectanglePointTopLeft = 0,
        RectanglePointTop,
        RectanglePointTopRight,
        RectanglePointRight,
        RectanglePointBottomRight,
        RectanglePointBottom,
        RectanglePointBottomLeft,
        RectanglePointLeft
    };

}
