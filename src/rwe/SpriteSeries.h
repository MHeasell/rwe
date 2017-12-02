#ifndef RWE_SPRITESERIES_H
#define RWE_SPRITESERIES_H

#include <memory>
#include <rwe/Sprite.h>
#include <vector>

namespace rwe
{
    struct SpriteSeries
    {
        std::vector<std::shared_ptr<Sprite>> sprites;
    };
}


#endif
