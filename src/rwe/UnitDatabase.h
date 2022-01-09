#pragma once

#include <boost/functional/hash.hpp>
#include <memory>
#include <rwe/UnitModelDefinition.h>
#include <rwe/io/cob/Cob.h>
#include <rwe/io/fbi/UnitFbi.h>
#include <rwe/io/gui/gui.h>
#include <rwe/sim/MovementClass.h>
#include <utility>

namespace rwe
{
    class UnitDatabase
    {
    public:
        using MovementClassIterator = typename std::unordered_map<std::string, MovementClass>::const_iterator;

    private:
        std::unordered_map<std::string, UnitFbi> map;

        std::unordered_map<std::string, CobScript> cobMap;

        std::unordered_map<std::string, MovementClass> movementClassMap;

        std::unordered_map<std::string, std::vector<std::vector<GuiEntry>>> builderGuisMap;

        std::unordered_map<std::string, UnitModelDefinition> unitModelDefinitionsMap;

    public:
        bool hasUnitInfo(const std::string& unitName) const;

        const UnitFbi& getUnitInfo(const std::string& unitName) const;

        void addUnitInfo(const std::string& unitName, const UnitFbi& info);

        const CobScript& getUnitScript(const std::string& unitName) const;

        void addUnitScript(const std::string& unitName, CobScript&& cob);

        const MovementClass& getMovementClass(const std::string& className) const;

        void addMovementClass(const std::string& className, MovementClass&& movementClass);

        std::optional<std::reference_wrapper<const std::vector<std::vector<GuiEntry>>>> tryGetBuilderGui(const std::string& unitName) const;

        void addBuilderGui(const std::string& unitName, std::vector<std::vector<GuiEntry>>&& gui);

        MovementClassIterator movementClassBegin() const;

        MovementClassIterator movementClassEnd() const;

        void addUnitModelDefinition(const std::string& objectName, UnitModelDefinition&& model);

        std::optional<std::reference_wrapper<const UnitModelDefinition>> getUnitModelDefinition(const std::string& objectName) const;

        bool hasUnitModelDefinition(const std::string& objectName) const;
    };
}
