#include "util.h"
#include <cmath>
#include <rwe/math/rwe_math.h>

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

    float toleranceToRadians(unsigned int angle)
    {
        return static_cast<float>(angle) * (Pif / 32768.0f);
    }
}
