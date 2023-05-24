#pragma once

#include <memory>
#include <rwe/io/gui/gui.h>
#include <string>
#include <utility>


namespace rwe
{
    class BuilderGuisDatabase
    {
    public:
        std::unordered_map<std::string, std::vector<std::vector<GuiEntry>>> builderGuisMap;

    public:
        std::optional<std::reference_wrapper<const std::vector<std::vector<GuiEntry>>>> tryGetBuilderGui(const std::string& unitName) const;

        void addBuilderGui(const std::string& unitName, std::vector<std::vector<GuiEntry>>&& gui);
    };
}
