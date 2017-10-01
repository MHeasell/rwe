#ifndef RWE_MAPFEATURE_H
#define RWE_MAPFEATURE_H

#include <boost/optional.hpp>
#include <memory>
#include <rwe/SpriteSeries.h>
#include <rwe/math/Vector3f.h>

namespace rwe
{
    struct MapFeature
    {
        std::shared_ptr<SpriteSeries> animation;
        boost::optional<std::shared_ptr<SpriteSeries>> shadowAnimation;
        Vector3f position;
        int footprintX;
        int footprintZ;
    };
}

#endif
