#include "util.h"
#include <cmath>

namespace rwe
{
#ifdef RWE_PLATFORM_WINDOWS
    std::optional<boost::filesystem::path> getLocalDataPath()
    {
        auto appData = std::getenv("APPDATA");
        if (appData == nullptr)
        {
            return std::nullopt;
        }

        boost::filesystem::path path(appData);
        path /= "RWE";

        return path;
    }
#endif

#ifdef RWE_PLATFORM_LINUX
    std::optional<boost::filesystem::path> getLocalDataPath()
    {
        auto home = std::getenv("HOME");
        if (home == nullptr)
        {
            return std::nullopt;
        }

        boost::filesystem::path path(home);
        path /= ".rwe";

        return path;
    }
#endif

    std::optional<boost::filesystem::path> getSearchPath()
    {
        auto path = getLocalDataPath();
        if (!path)
        {
            return std::nullopt;
        }


        *path /= "Data";

        return *path;
    }

    float toRadians(float v)
    {
        return v * (Pif / 180.0f);
    }

    RadiansAngle toRadians(CobAngle angle)
    {
        return RadiansAngle::fromUnwrappedAngle(static_cast<float>(angle.value) * (Pif / 32768.0f));
    }

    float toleranceToRadians(int angle)
    {
        return static_cast<float>(angle) * (Pif / 32768.0f);
    }

    CobAngle toCobAngle(RadiansAngle angle)
    {
        return CobAngle(static_cast<uint16_t>(std::round(angle.value * (32768.0f / Pif))));
    }

    float angleLerp(float a, float b, float t)
    {
        if (b - a >= Pif)
        {
            return rweLerp(a + (2.0f * Pif), b, t);
        }
        if (b - a < -Pif)
        {
            return rweLerp(a, b + (2.0f * Pif), t);
        }
        return rweLerp(a, b, t);
    }
}
