#include "UnitDatabase.h"
#include <rwe/rwe_string.h>

namespace rwe
{
    std::optional<std::reference_wrapper<const std::vector<std::vector<GuiEntry>>>>
    UnitDatabase::tryGetBuilderGui(const std::string& unitName) const
    {
        auto it = builderGuisMap.find(unitName);
        if (it == builderGuisMap.end())
        {
            return std::nullopt;
        }

        return it->second;
    }

    void UnitDatabase::addBuilderGui(const std::string& unitName, std::vector<std::vector<GuiEntry>>&& gui)
    {
        builderGuisMap.insert({unitName, std::move(gui)});
    }

    bool UnitDatabase::hasFeature(const std::string& featureName) const
    {
        return featureNameIndex.find(toUpper(featureName)) != featureNameIndex.end();
    }

    std::optional<FeatureDefinitionId> UnitDatabase::tryGetFeatureId(const std::string& featureName) const
    {
        if (auto it = featureNameIndex.find(toUpper(featureName)); it != featureNameIndex.end())
        {
            return it->second;
        }

        return std::nullopt;
    }

    const FeatureDefinition& UnitDatabase::getFeature(FeatureDefinitionId id) const
    {
        return featureMap.get(id);
    }

    FeatureDefinition& UnitDatabase::getFeature(FeatureDefinitionId id)
    {
        return featureMap.get(id);
    }

    FeatureDefinitionId UnitDatabase::addFeature(const std::string& featureName, const FeatureDefinition& definition)
    {
        auto id = featureMap.insert(definition);
        featureNameIndex.insert({toUpper(featureName), id});
        return id;
    }

    FeatureDefinitionId UnitDatabase::getNextFeatureDefinitionId() const
    {
        return featureMap.getNextId();
    }
}
