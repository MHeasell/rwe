#include "MapFeatureService.h"
#include "tdf.h"

namespace rwe
{
    void MapFeatureService::loadAllFeatureDefinitions()
    {
        auto files = vfs->getFileNamesRecursive("features", ".tdf");

        for (const auto& name : files)
        {
            auto bytes = vfs->readFile("features/" + name);
            if (!bytes)
            {
                throw std::runtime_error("Failed to read feature " + name);
            }

            std::string tdfString(bytes->data(), bytes->size());

            auto tdfRoot = parseTdfFromString(tdfString);
            for (const auto& e : tdfRoot.entries)
            {
                auto block = boost::get<TdfBlock>(e.value.get());
                if (block != nullptr)
                {
                    auto featureDefinition = FeatureDefinition::fromTdf(*block);
                    features.insert_or_assign(e.name, std::move(featureDefinition));
                }
            }
        }
    }

    const FeatureDefinition& MapFeatureService::getFeatureDefinition(const std::string& featureName)
    {
        auto it = features.find(featureName);
        if (it == features.end())
        {
            throw std::runtime_error("Failed to find feature definition: " + featureName);
        }

        return it->second;
    }

    MapFeatureService::MapFeatureService(AbstractVirtualFileSystem* vfs) : vfs(vfs)
    {}
}
