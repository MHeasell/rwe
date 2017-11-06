#include <rwe/math/rwe_math.h>
#include "ViewportService.h"

namespace rwe
{
    ViewportService::ViewportService(unsigned int width, unsigned int height) : _width(width), _height(height)
    {
    }

    Vector2f ViewportService::toClipSpace(int x, int y) const
    {
        return toClipSpace(Point(x, y));
    }

    Vector2f ViewportService::toClipSpace(Point p) const
    {
        return convertScreenToClipSpace(_width, _height, p);
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
