#pragma once

namespace QSchematic
{

    /**
     * Text direction.
     */
    enum class Direction
    {
        LeftToRight = 0,
        RightToLeft,
        TopToBottom,
        BottomToTop
    };

    /**
     * Enum to identify eight different points on a rectangle.
     */
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
