#ifndef RWE_SKIRMISHMENUMODEL_H
#define RWE_SKIRMISHMENUMODEL_H

#include <boost/optional.hpp>
#include <rwe/observable/BehaviorSubject.h>
#include <string>

namespace rwe
{
    class SkirmishMenuModel
    {
    public:
        struct SelectedMapInfo
        {
            std::string name;
            std::string description;
            std::string size;

            bool operator==(const SelectedMapInfo& rhs) const;

            bool operator!=(const SelectedMapInfo& rhs) const;
        };

    public:
        BehaviorSubject<boost::optional<SelectedMapInfo>> selectedMap;

    };
}

#endif
