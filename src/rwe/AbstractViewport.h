#pragma once

#include <rwe/grid/Point.h>
#include <rwe/math/Vector2f.h>
namespace rwe
{
    class AbstractViewport
    {
    public:
        Vector2f toClipSpace(int x, int y) const;

        Vector2f toClipSpace(Point p) const;

        Point toViewportSpace(float x, float y) const;

        Point toViewportSpace(Vector2f v) const;

        Point toOtherViewport(const AbstractViewport& v, int x, int y);

        Point toOtherViewport(const AbstractViewport& v, const Point& p);

        virtual int x() const = 0;

        virtual int y() const = 0;

        virtual unsigned int width() const = 0;

        virtual unsigned int height() const = 0;

        int top() const { return y(); }
        int bottom() const { return y() + static_cast<int>(height()); }
        int left() const { return x(); }
        int right() const { return x() + static_cast<int>(width()); }

        bool contains(int px, int py) const;
        bool contains(const Point& p) const;
    };
}
