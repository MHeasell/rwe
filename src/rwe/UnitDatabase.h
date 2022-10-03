#pragma once

#include <boost/functional/hash.hpp>
#include <memory>
#include <rwe/SimpleVectorMap.h>
#include <rwe/io/cob/Cob.h>
#include <rwe/io/fbi/UnitFbi.h>
#include <rwe/io/gui/gui.h>
#include <rwe/sim/FeatureDefinition.h>
#include <rwe/sim/MovementClass.h>
#include <rwe/sim/UnitModelDefinition.h>
#include <utility>


namespace rwe
{
    class UnitDatabase
    {
    public:
        using MovementClassIterator = typename std::unordered_map<std::string, MovementClass>::const_iterator;

    public:
        std::unordered_map<std::string, std::vector<std::vector<GuiEntry>>> builderGuisMap;

        SimpleVectorMap<FeatureDefinition, FeatureDefinitionIdTag> featureMap;
        std::unordered_map<std::string, FeatureDefinitionId> featureNameIndex;

    public:
        std::optional<std::reference_wrapper<const std::vector<std::vector<GuiEntry>>>> tryGetBuilderGui(const std::string& unitName) const;

        void addBuilderGui(const std::string& unitName, std::vector<std::vector<GuiEntry>>&& gui);

        bool hasFeature(const std::string& featureName) const;

        std::optional<FeatureDefinitionId> tryGetFeatureId(const std::string& featureName) const;

        const FeatureDefinition& getFeature(FeatureDefinitionId id) const;

        FeatureDefinition& getFeature(FeatureDefinitionId id);

        FeatureDefinitionId addFeature(const std::string& featureName, const FeatureDefinition& definition);

        FeatureDefinitionId getNextFeatureDefinitionId() const;
    };
}
