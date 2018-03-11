#ifndef RWE_MAINMENUMODEL_H
#define RWE_MAINMENUMODEL_H

#include <array>
#include <boost/variant.hpp>
#include <optional>
#include <rwe/Sprite.h>
#include <rwe/events.h>
#include <rwe/observable/BehaviorSubject.h>
#include <string>

namespace rwe
{
    class MainMenuModel
    {
    public:
        struct SelectedMapInfo
        {
            std::string name;
            std::string description;
            std::string size;

            std::shared_ptr<Sprite> minimap;

            SelectedMapInfo(
                const std::string& name,
                const std::string& description,
                const std::string& size,
                const std::shared_ptr<Sprite>& minimap);

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
            BehaviorSubject<unsigned int> colorIndex;
            BehaviorSubject<std::optional<int>> teamIndex;
            BehaviorSubject<int> metal;
            BehaviorSubject<int> energy;
        };

    public:
        Subject<GroupMessage> groupMessages;

        Subject<int> teamChanges;

        BehaviorSubject<std::optional<SelectedMapInfo>> selectedMap;

        BehaviorSubject<std::optional<SelectedMapInfo>> candidateSelectedMap;

        std::array<PlayerSettings, 10> players{{
            {PlayerSettings::Type::Human, PlayerSettings::Side::Arm, 0, std::optional<int>(std::nullopt), 1000, 1000},
            {PlayerSettings::Type::Computer, PlayerSettings::Side::Core, 1, std::optional<int>(std::nullopt), 1000, 1000},
            {PlayerSettings::Type::Open, PlayerSettings::Side::Arm, 2, std::optional<int>(std::nullopt), 1000, 1000},
            {PlayerSettings::Type::Open, PlayerSettings::Side::Core, 3, std::optional<int>(std::nullopt), 1000, 1000},
            {PlayerSettings::Type::Open, PlayerSettings::Side::Arm, 4, std::optional<int>(std::nullopt), 1000, 1000},
            {PlayerSettings::Type::Open, PlayerSettings::Side::Core, 5, std::optional<int>(std::nullopt), 1000, 1000},
            {PlayerSettings::Type::Open, PlayerSettings::Side::Arm, 6, std::optional<int>(std::nullopt), 1000, 1000},
            {PlayerSettings::Type::Open, PlayerSettings::Side::Core, 7, std::optional<int>(std::nullopt), 1000, 1000},
            {PlayerSettings::Type::Open, PlayerSettings::Side::Arm, 8, std::optional<int>(std::nullopt), 1000, 1000},
            {PlayerSettings::Type::Open, PlayerSettings::Side::Core, 9, std::optional<int>(std::nullopt), 1000, 1000},
        }};

        /**
         * Returns true when the team contains two or more players.
         */
        bool isTeamShared(int index) const;

        /**
         * Returns true if there is a player using the given color index.
         */
        bool isColorInUse(unsigned int colorIndex) const;

        /**
         * Returns the first available player color index.
         */
        std::optional<unsigned int> getFirstFreeColor() const;
    };
}

#endif
