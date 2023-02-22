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
}
