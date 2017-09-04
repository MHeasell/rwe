#include "SkirmishMenuModel.h"

namespace rwe
{
    bool SkirmishMenuModel::SelectedMapInfo::operator==(const SkirmishMenuModel::SelectedMapInfo& rhs) const
    {
        return name == rhs.name && description == rhs.description && size == rhs.size;
    }

    bool SkirmishMenuModel::SelectedMapInfo::operator!=(const SkirmishMenuModel::SelectedMapInfo& rhs) const
    {
        return !(rhs == *this);
    }
}
