#ifndef RWE_SKIRMISHMENUMODEL_H
#define RWE_SKIRMISHMENUMODEL_H

#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <rwe/Sprite.h>
#include <rwe/events.h>
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

            Sprite minimap;

            SelectedMapInfo(
                const std::string& name,
                const std::string& description,
                const std::string& size,
                const Sprite& minimap);

            bool operator==(const SelectedMapInfo& rhs) const;

            bool operator!=(const SelectedMapInfo& rhs) const;
        };

    public:
        Subject<GroupMessage> groupMessages;

        BehaviorSubject<boost::optional<SelectedMapInfo>> selectedMap;

        BehaviorSubject<boost::optional<SelectedMapInfo>> candidateSelectedMap;
    };
}

#endif
