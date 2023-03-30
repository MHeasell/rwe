#pragma once

#include <boost/filesystem/path.hpp>
#include <optional>
#include <rwe/RadiansAngle.h>
#include <rwe/cob/CobAngle.h>
#include <rwe/float_math.h>

namespace rwe
{
    std::optional<boost::filesystem::path> getLocalDataPath();
    std::optional<boost::filesystem::path> getSearchPath();

    RadiansAngle toRadians(CobAngle angle);

    float toleranceToRadians(unsigned int angle);

    CobAngle toCobAngle(RadiansAngle angle);
}
