#pragma once

#include <rwe/grid/Point.h>
#include <rwe/math/Vector2f.h>

namespace rwe
{
    class Viewport
    {
    private:
        int _x;
        int _y;
        int _width;
        int _height;

    public:
        Viewport(int x, int y, int width, int height);

        Vector2f toClipSpace(int x, int y) const;

        Vector2f toClipSpace(Point p) const;

        Point toViewportSpace(float x, float y) const;

        Point toViewportSpace(Vector2f v) const;

        Point toOtherViewport(const Viewport& v, int x, int y);

        Point toOtherViewport(const Viewport& v, const Point& p);

        int x() const;

        int y() const;

        int width() const;

        int height() const;

        int top() const { return _y; }
        int bottom() const { return _y + static_cast<int>(_height); }
        int left() const { return _x; }
        int right() const { return _x + static_cast<int>(_width); }

        bool contains(int x, int y) const;
        bool contains(const Point& p) const;
    };
}
