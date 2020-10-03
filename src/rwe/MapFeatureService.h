#pragma once

#include <rwe/io/featuretdf/FeatureDefinition.h>
#include <rwe/vfs/AbstractVirtualFileSystem.h>
#include <string>

namespace rwe
{
    class MapFeatureService
    {
    private:
        AbstractVirtualFileSystem* vfs;

        std::unordered_map<std::string, FeatureDefinition> features;

    public:
        MapFeatureService(AbstractVirtualFileSystem* vfs);

        void loadAllFeatureDefinitions();

        const FeatureDefinition& getFeatureDefinition(const std::string& featureName);
    };
}
