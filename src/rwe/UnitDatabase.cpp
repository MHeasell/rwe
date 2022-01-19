#include "UnitDatabase.h"
#include <rwe/rwe_string.h>

namespace rwe
{
    bool UnitDatabase::hasUnitInfo(const std::string& unitName) const
    {
        return map.find(toUpper(unitName)) != map.end();
    }

    const UnitFbi& UnitDatabase::getUnitInfo(const std::string& unitName) const
    {
        auto it = map.find(toUpper(unitName));
        if (it == map.end())
        {
            throw std::runtime_error("No FBI data found for unit " + unitName);
        }

        return it->second;
    }

    void UnitDatabase::addUnitInfo(const std::string& unitName, const UnitFbi& info)
    {
        map.insert({toUpper(unitName), info});
    }

    const CobScript& UnitDatabase::getUnitScript(const std::string& unitName) const
    {
        auto it = cobMap.find(toUpper(unitName));
        if (it == cobMap.end())
        {
            throw std::runtime_error("No script data found for unit " + unitName);
        }

        return it->second;
    }

    void UnitDatabase::addUnitScript(const std::string& unitName, CobScript&& cob)
    {
        cobMap.insert({toUpper(unitName), std::move(cob)});
    }

    const MovementClass& UnitDatabase::getMovementClass(const std::string& className) const
    {
        auto it = movementClassMap.find(className);
        if (it == movementClassMap.end())
        {
            throw std::runtime_error("No movement class found with name " + className);
        }

        return it->second;
    }

    void UnitDatabase::addMovementClass(const std::string& className, MovementClass&& movementClass)
    {
        movementClassMap.insert({className, std::move(movementClass)});
    }

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

    UnitDatabase::MovementClassIterator UnitDatabase::movementClassBegin() const
    {
        return movementClassMap.begin();
    }

    UnitDatabase::MovementClassIterator UnitDatabase::movementClassEnd() const
    {
        return movementClassMap.end();
    }

    void UnitDatabase::addUnitModelDefinition(const std::string& objectName, UnitModelDefinition&& model)
    {
        unitModelDefinitionsMap.insert({objectName, std::move(model)});
    }

    std::optional<std::reference_wrapper<const UnitModelDefinition>> UnitDatabase::getUnitModelDefinition(const std::string& objectName) const
    {
        auto it = unitModelDefinitionsMap.find(objectName);
        if (it == unitModelDefinitionsMap.end())
        {
            return std::nullopt;
        }
        return it->second;
    }

    bool UnitDatabase::hasUnitModelDefinition(const std::string& objectName) const
    {
        auto it = unitModelDefinitionsMap.find(objectName);
        return it != unitModelDefinitionsMap.end();
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
