#ifndef RWE_SKIRMISHMENUMODEL_H
#define RWE_SKIRMISHMENUMODEL_H

#include <array>
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

        struct PlayerSettings
        {
            enum class Type
            {
                Open,
                Human,
                Computer
            };
            enum class Side
            {
                Arm,
                Core
            };

            BehaviorSubject<Type> type;
            BehaviorSubject<Side> side;
            BehaviorSubject<int> colorIndex;
            BehaviorSubject<boost::optional<int>> teamIndex;
            BehaviorSubject<int> metal;
            BehaviorSubject<int> energy;
        };

    public:
        Subject<GroupMessage> groupMessages;

        Subject<int> teamChanges;

        BehaviorSubject<boost::optional<SelectedMapInfo>> selectedMap;

        BehaviorSubject<boost::optional<SelectedMapInfo>> candidateSelectedMap;

        std::array<PlayerSettings, 10> players{{
            {PlayerSettings::Type::Human, PlayerSettings::Side::Arm, 0, boost::optional<int>(boost::none), 1000, 1000},
            {PlayerSettings::Type::Computer, PlayerSettings::Side::Core, 1, boost::optional<int>(boost::none), 1000, 1000},
            {PlayerSettings::Type::Open, PlayerSettings::Side::Arm, 2, boost::optional<int>(boost::none), 1000, 1000},
            {PlayerSettings::Type::Open, PlayerSettings::Side::Core, 3, boost::optional<int>(boost::none), 1000, 1000},
            {PlayerSettings::Type::Open, PlayerSettings::Side::Arm, 4, boost::optional<int>(boost::none), 1000, 1000},
            {PlayerSettings::Type::Open, PlayerSettings::Side::Core, 5, boost::optional<int>(boost::none), 1000, 1000},
            {PlayerSettings::Type::Open, PlayerSettings::Side::Arm, 6, boost::optional<int>(boost::none), 1000, 1000},
            {PlayerSettings::Type::Open, PlayerSettings::Side::Core, 7, boost::optional<int>(boost::none), 1000, 1000},
            {PlayerSettings::Type::Open, PlayerSettings::Side::Arm, 8, boost::optional<int>(boost::none), 1000, 1000},
            {PlayerSettings::Type::Open, PlayerSettings::Side::Core, 9, boost::optional<int>(boost::none), 1000, 1000},
        }};

        /**
         * Returns true when the team contains two or more players.
         */
        bool isTeamShared(int index) const;

        /**
         * Returns true if there is a player using the given color index.
         */
        bool isColorInUse(int colorIndex) const;

        /**
         * Returns the first available player color index.
         */
        boost::optional<int> getFirstFreeColor() const;
    };
}

#endif
