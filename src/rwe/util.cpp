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

    boost::optional<boost::filesystem::path> getSearchPath()
    {
        auto path = getLocalDataPath();
        if (!path)
        {
            return boost::none;
        }


        *path /= "Data";

        return *path;
    }

    float toRadians(float v)
    {
        return v * (Pif / 180.0f);
    }

    Radians toRadians(TaAngle angle)
    {
        return Radians(static_cast<float>(angle.value) * (Pif / 32760.0f));
    }
}
