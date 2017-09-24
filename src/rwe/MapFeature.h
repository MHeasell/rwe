#ifndef RWE_MAPFEATURE_H
#define RWE_MAPFEATURE_H

#include <memory>
#include <rwe/SpriteSeries.h>
#include <rwe/math/Vector3f.h>

namespace rwe
{
    struct MapFeature
    {
        std::shared_ptr<SpriteSeries> animation;
        Vector3f position;
        int footprintX;
        int footprintZ;
    };
}

#endif
