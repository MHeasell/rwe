#pragma once

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
