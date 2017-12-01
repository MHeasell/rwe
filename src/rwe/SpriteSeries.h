#ifndef RWE_SPRITESERIES_H
#define RWE_SPRITESERIES_H

#include <rwe/Sprite.h>
#include <vector>
#include <memory>

namespace rwe
{
    struct SpriteSeries
    {
        std::vector<std::shared_ptr<Sprite>> sprites;
    };
}


#endif
