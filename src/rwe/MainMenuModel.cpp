#include "MainMenuModel.h"

namespace rwe
{
    bool MainMenuModel::SelectedMapInfo::operator==(const MainMenuModel::SelectedMapInfo& rhs) const
    {
        return name == rhs.name
            && description == rhs.description
            && size == rhs.size
            && minimap == rhs.minimap;
    }

    bool MainMenuModel::SelectedMapInfo::operator!=(const MainMenuModel::SelectedMapInfo& rhs) const
    {
        return !(rhs == *this);
    }

    MainMenuModel::SelectedMapInfo::SelectedMapInfo(
        const std::string& name,
        const std::string& description,
        const std::string& size,
        const std::shared_ptr<Sprite>& minimap)
        : name(name),
          description(description),
          size(size),
          minimap(minimap)
    {
    }

    bool MainMenuModel::isTeamShared(int index) const
    {
        auto count = std::count_if(players.begin(), players.end(), [index](const auto& p) {
            const auto& val = p.teamIndex.getValue();
            return val && *val == index;
        });

        return count >= 2;
    }

    bool MainMenuModel::isColorInUse(unsigned int colorIndex) const
    {
        return std::find_if(players.begin(), players.end(), [colorIndex](const auto& p) {
            return p.type.getValue() != PlayerSettings::Type::Open && p.colorIndex.getValue() == colorIndex;
        }) != players.end();
    }

    std::optional<unsigned int> MainMenuModel::getFirstFreeColor() const
    {
        for (unsigned int i = 0; i < 10; ++i)
        {
            if (!isColorInUse(i))
            {
                return i;
            }
        }

        return std::nullopt;
    }
}
