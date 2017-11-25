#ifndef RWE_VIEWPORTSERVICE_H
#define RWE_VIEWPORTSERVICE_H

#include <rwe/Point.h>
#include <rwe/math/Vector2f.h>

namespace rwe
{
    class ViewportService
    {
    private:
        unsigned int _width;
        unsigned int _height;

    public:
        ViewportService(unsigned int width, unsigned int height);

        Vector2f toClipSpace(int x, int y) const;

        Vector2f toClipSpace(Point p) const;

        unsigned int width() const;

        unsigned int height() const;
    };
}

#endif
