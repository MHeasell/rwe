#ifndef RWE_MAPFEATURESERVICE_H
#define RWE_MAPFEATURESERVICE_H

#include <string>
#include <rwe/vfs/AbstractVirtualFileSystem.h>
#include <rwe/FeatureDefinition.h>

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

#endif
