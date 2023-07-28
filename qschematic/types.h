#pragma once

namespace QSchematic
{

    enum class Direction
    {
        LeftToRight = 0,
        RightToLeft,
        TopToBottom,
        BottomToTop
    };

    enum class RectanglePoint
    {
        TopLeft = 0,
        Top,
        TopRight,
        Right,
        BottomRight,
        Bottom,
        BottomLeft,
        Left,
    };

}
