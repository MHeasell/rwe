#include "UnitDatabase.h"
#include <rwe/rwe_string.h>

namespace rwe
{
    bool UnitDatabase::hasUnitInfo(const std::string& unitName) const
    {
        return map.find(toUpper(unitName)) != map.end();
    }

    const UnitFbi& UnitDatabase::getUnitInfo(const std::string& unitName) const
    {
        auto it = map.find(toUpper(unitName));
        if (it == map.end())
        {
            throw std::runtime_error("No FBI data found for unit " + unitName);
        }

        return it->second;
    }

    void UnitDatabase::addUnitInfo(const std::string& unitName, const UnitFbi& info)
    {
        map.insert({toUpper(unitName), info});
    }

    const CobScript& UnitDatabase::getUnitScript(const std::string& unitName) const
    {
        auto it = cobMap.find(toUpper(unitName));
        if (it == cobMap.end())
        {
            throw std::runtime_error("No script data found for unit " + unitName);
        }

        return it->second;
    }

    void UnitDatabase::addUnitScript(const std::string& unitName, CobScript&& cob)
    {
        cobMap.insert({toUpper(unitName), std::move(cob)});
    }

    const WeaponTdf& UnitDatabase::getWeapon(const std::string& weaponName) const
    {
        auto it = weaponMap.find(toUpper(weaponName));
        if (it == weaponMap.end())
        {
            throw std::runtime_error("No weapon found with name " + weaponName);
        }

        return it->second;
    }

    void UnitDatabase::addWeapon(const std::string& weaponName, WeaponTdf&& weapon)
    {
        weaponMap.insert({toUpper(weaponName), std::move(weapon)});
    }

    const SoundClass& UnitDatabase::getSoundClass(const std::string& className) const
    {
        auto it = soundClassMap.find(className);
        if (it == soundClassMap.end())
        {
            throw std::runtime_error("No sound class found with name " + className);
        }

        return it->second;
    }

    void UnitDatabase::addSoundClass(const std::string& className, SoundClass&& soundClass)
    {
        soundClassMap.insert({className, std::move(soundClass)});
    }

    const AudioService::SoundHandle& UnitDatabase::getSoundHandle(const std::string& sound) const
    {
        auto it = soundMap.find(sound);
        if (it == soundMap.end())
        {
            throw std::runtime_error("No sound found with name " + sound);
        }

        return it->second;
    }

    std::optional<AudioService::SoundHandle> UnitDatabase::tryGetSoundHandle(const std::string& sound)
    {
        auto it = soundMap.find(sound);
        if (it == soundMap.end())
        {
            return std::nullopt;
        }

        return it->second;
    }

    void UnitDatabase::addSound(const std::string& soundName, const AudioService::SoundHandle& sound)
    {
        soundMap.insert({soundName, sound});
    }

    const MovementClass& UnitDatabase::getMovementClass(const std::string& className) const
    {
        auto it = movementClassMap.find(className);
        if (it == movementClassMap.end())
        {
            throw std::runtime_error("No movement class found with name " + className);
        }

        return it->second;
    }

    void UnitDatabase::addMovementClass(const std::string& className, MovementClass&& movementClass)
    {
        movementClassMap.insert({className, std::move(movementClass)});
    }

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

    UnitDatabase::MovementClassIterator UnitDatabase::movementClassBegin() const
    {
        return movementClassMap.begin();
    }

    UnitDatabase::MovementClassIterator UnitDatabase::movementClassEnd() const
    {
        return movementClassMap.end();
    }
}
