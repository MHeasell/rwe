#include "AbstractViewport.h"

namespace rwe
{
    Vector2f AbstractViewport::toClipSpace(int x, int y) const
    {
        return toClipSpace(Point(x, y));
    }

    Vector2f AbstractViewport::toClipSpace(Point p) const
    {
        auto halfScreenWidth = static_cast<float>(width()) / 2.0f;
        auto halfScreenHeight = static_cast<float>(height()) / 2.0f;

        auto x = (static_cast<float>(p.x) / halfScreenWidth) - 1.0f;
        auto y = (static_cast<float>(height() - p.y) / halfScreenHeight) - 1.0f;
        return Vector2f(x, y);
    }

    Point AbstractViewport::toViewportSpace(float x, float y) const
    {
        return toViewportSpace(Vector2f(x, y));
    }

    Point AbstractViewport::toViewportSpace(Vector2f v) const
    {
        auto halfScreenWidth = static_cast<float>(width()) / 2.0f;
        auto halfScreenHeight = static_cast<float>(height()) / 2.0f;

        auto x = static_cast<int>((v.x + 1.0f) * halfScreenWidth);
        auto y = height() - static_cast<int>((v.y + 1.0f) * halfScreenHeight);
        return Point(x, y);
    }

    Point AbstractViewport::toOtherViewport(const AbstractViewport& v, int x, int y)
    {
        return toOtherViewport(v, Point(x, y));
    }

    /**
     * Transforms the given pixel coordinate in this viewport
     * to the pixel coordinate in the other viewport
     * that corresponds to the same position on the screen.
     */
    Point AbstractViewport::toOtherViewport(const AbstractViewport& v, const Point& p)
    {
        return Point(p.x + x() - v.x(), p.y + y() - v.y());
    }

    bool AbstractViewport::contains(int px, int py) const
    {
        return px >= x() && px < (x() + static_cast<int>(width())) && py >= y() && py < (y() + static_cast<int>(height()));
    }

    bool AbstractViewport::contains(const Point& p) const
    {
        return contains(p.x, p.y);
    }
}
