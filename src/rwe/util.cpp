#include "util.h"

namespace rwe
{
    boost::optional<boost::filesystem::path> getSearchPath()
    {
        auto appData = std::getenv("APPDATA");
        if (appData == nullptr)
        {
            return boost::none;
        }

        boost::filesystem::path searchPath(appData);
        searchPath /= "RWE";
        searchPath /= "Data";

        return searchPath;
    }

    float toRadians(float v)
    {
        return v * (3.14159265358979323846f / 180.0f);
    }
}
