#include "SkirmishMenuModel.h"

namespace rwe
{
    bool SkirmishMenuModel::SelectedMapInfo::operator==(const SkirmishMenuModel::SelectedMapInfo& rhs) const
    {
        return name == rhs.name
            && description == rhs.description
            && size == rhs.size
            && minimap == rhs.minimap;
    }

    bool SkirmishMenuModel::SelectedMapInfo::operator!=(const SkirmishMenuModel::SelectedMapInfo& rhs) const
    {
        return !(rhs == *this);
    }

    SkirmishMenuModel::SelectedMapInfo::SelectedMapInfo(
        const std::string& name,
        const std::string& description,
        const std::string& size,
        const Sprite& minimap)
        : name(name),
          description(description),
          size(size),
          minimap(minimap)
    {
    }

    bool SkirmishMenuModel::isTeamShared(int index) const
    {
        auto count = std::count_if(players.begin(), players.end(), [index](const auto& p) {
            const auto& val = p.teamIndex.getValue();
            return val && *val == index;
        });

        return count >= 2;
    }

    bool SkirmishMenuModel::isColorInUse(int colorIndex) const
    {
        return std::find_if(players.begin(), players.end(), [colorIndex](const auto& p) {
            return p.type.getValue() != PlayerSettings::Type::Open && p.colorIndex.getValue() == colorIndex;
        }) != players.end();
    }

    boost::optional<int> SkirmishMenuModel::getFirstFreeColor() const
    {
        for (int i = 0; i < 10; ++i)
        {
            if (!isColorInUse(i))
            {
                return i;
            }
        }

        return boost::none;
    }
}
