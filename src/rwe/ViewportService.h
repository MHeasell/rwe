#pragma once

#include <rwe/Point.h>
#include <rwe/math/Vector2f.h>

namespace rwe
{
    class ViewportService
    {
    private:
        int _x;
        int _y;
        unsigned int _width;
        unsigned int _height;

    public:
        ViewportService(int x, int y, unsigned int width, unsigned int height);

        Vector2f toClipSpace(int x, int y) const;

        Vector2f toClipSpace(Point p) const;

        Point toViewportSpace(float x, float y) const;

        Point toViewportSpace(Vector2f v) const;

        Point toOtherViewport(const ViewportService& v, int x, int y);

        Point toOtherViewport(const ViewportService& v, const Point& p);

        int x() const;

        int y() const;

        unsigned int width() const;

        unsigned int height() const;

        int top() const { return _y; }
        int bottom() const { return _y + static_cast<int>(_height); }
        int left() const { return _x; }
        int right() const { return _x + static_cast<int>(_width); }

        bool contains(int x, int y) const;
        bool contains(const Point& p) const;
    };
}
