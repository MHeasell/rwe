#include "util.h"

namespace rwe
{
    boost::optional<boost::filesystem::path> getLocalDataPath()
    {
        auto appData = std::getenv("APPDATA");
        if (appData == nullptr)
        {
            return boost::none;
        }

        boost::filesystem::path path(appData);
        path /= "RWE";

        return path;
    }

    float toRadians(float v)
    {
        return v * (3.14159265358979323846f / 180.0f);
    }
}
