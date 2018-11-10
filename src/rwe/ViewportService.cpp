#include "ViewportService.h"
#include <rwe/math/rwe_math.h>

namespace rwe
{
    ViewportService::ViewportService(int x, int y, unsigned int width, unsigned int height) : _x(x), _y(y), _width(width), _height(height)
    {
    }

    Vector2f ViewportService::toClipSpace(int x, int y) const
    {
        return toClipSpace(Point(x, y));
    }

    Vector2f ViewportService::toClipSpace(Point p) const
    {
        auto halfScreenWidth = static_cast<float>(_width) / 2.0f;
        auto halfScreenHeight = static_cast<float>(_height) / 2.0f;

        auto x = (static_cast<float>(p.x) / halfScreenWidth) - 1.0f;
        auto y = (static_cast<float>(_height - p.y) / halfScreenHeight) - 1.0f;
        return Vector2f(x, y);
    }

    Point ViewportService::toViewportSpace(float x, float y) const
    {
        return toViewportSpace(Vector2f(x, y));
    }

    Point ViewportService::toViewportSpace(Vector2f v) const
    {
        auto halfScreenWidth = static_cast<float>(_width) / 2.0f;
        auto halfScreenHeight = static_cast<float>(_height) / 2.0f;

        auto x = static_cast<int>((v.x + 1.0f) * halfScreenWidth);
        auto y = _height - static_cast<int>((v.y + 1.0f) * halfScreenHeight);
        return Point(x, y);
    }

    Point ViewportService::toOtherViewport(const ViewportService& v, int x, int y)
    {
        return toOtherViewport(v, Point(x, y));
    }

    /**
     * Transforms the given pixel coordinate in this viewport
     * to the pixel coordinate in the other viewport
     * that corresponds to the same position on the screen.
     */
    Point ViewportService::toOtherViewport(const ViewportService& v, const Point& p)
    {
        return Point(p.x + _x - v._x, p.y + _y - v._y);
    }

    unsigned int ViewportService::x() const
    {
        return _x;
    }

    unsigned int ViewportService::y() const
    {
        return _y;
    }

    unsigned int ViewportService::width() const
    {
        return _width;
    }

    unsigned int ViewportService::height() const
    {
        return _height;
    }
}
