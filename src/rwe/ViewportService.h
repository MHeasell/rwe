#ifndef RWE_VIEWPORTSERVICE_H
#define RWE_VIEWPORTSERVICE_H

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

        unsigned int x() const;

        unsigned int y() const;

        unsigned int width() const;

        unsigned int height() const;
    };
}

#endif
