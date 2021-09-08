#pragma once

#include <rwe/AbstractViewport.h>
#include <rwe/grid/Point.h>
#include <rwe/math/Vector2f.h>

namespace rwe
{
    class Viewport : public AbstractViewport
    {
    private:
        int _x;
        int _y;
        unsigned int _width;
        unsigned int _height;

    public:
        Viewport(int x, int y, unsigned int width, unsigned int height);

        int x() const override;

        int y() const override;

        unsigned int width() const override;

        unsigned int height() const override;
    };
}
