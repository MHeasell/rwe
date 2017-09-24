#include "MapFeatureService.h"

namespace rwe
{
    void MapFeatureService::loadAllFeatureDefinitions()
    {
        auto files = vfs->getFileNamesRecursive("features", ".tdf");

        for (const auto& name : files)
        {
            auto bytes = vfs->readFile("features/" + name + ".tdf");
            if (!bytes)
            {
                throw std::runtime_error("Failed to read feature " + name);
            }


        }
    }
}
