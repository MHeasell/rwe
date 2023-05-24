#include "GameMediaDatabase.h"

namespace rwe
{
    void GameMediaDatabase::addSelectionMesh(const std::string& objectName, std::shared_ptr<GlMesh> mesh)
    {
        selectionMeshesMap.insert({objectName, mesh});
    }

    std::optional<std::shared_ptr<GlMesh>> GameMediaDatabase::getSelectionMesh(const std::string& objectName) const
    {
        auto it = selectionMeshesMap.find(objectName);
        if (it == selectionMeshesMap.end())
        {
            return std::nullopt;
        }
        return it->second;
    }

    void GameMediaDatabase::addUnitPieceMesh(const std::string& unitName, const std::string& pieceName, const UnitPieceMeshInfo& pieceMesh)
    {
        unitPieceMeshesMap.insert({{unitName, pieceName}, pieceMesh});
    }

    std::optional<std::reference_wrapper<const UnitPieceMeshInfo>> GameMediaDatabase::getUnitPieceMesh(const std::string& objectName, const std::string& pieceName) const
    {
        auto it = unitPieceMeshesMap.find({objectName, pieceName});
        if (it == unitPieceMeshesMap.end())
        {
            return std::nullopt;
        }
        return it->second;
    }

    void GameMediaDatabase::addSpriteSeries(const std::string& gafName, const std::string& animName, std::shared_ptr<SpriteSeries> sprite)
    {
        spritesMap.insert({{gafName, animName}, sprite});
    }

    std::optional<std::shared_ptr<SpriteSeries>> GameMediaDatabase::getSpriteSeries(const std::string& gaf, const std::string& anim) const
    {
        auto it = spritesMap.find({gaf, anim});
        if (it == spritesMap.end())
        {
            return std::nullopt;
        }
        return it->second;
    }

    const WeaponMediaInfo& GameMediaDatabase::getWeapon(const std::string& weaponName) const
    {
        auto it = weaponMap.find(toUpper(weaponName));
        if (it == weaponMap.end())
        {
            throw std::runtime_error("No weapon found with name " + weaponName);
        }

        return it->second;
    }

    std::optional<std::reference_wrapper<const WeaponMediaInfo>> GameMediaDatabase::tryGetWeapon(const std::string& weaponName) const
    {
        auto it = weaponMap.find(toUpper(weaponName));
        if (it == weaponMap.end())
        {
            return std::nullopt;
        }

        return it->second;
    }

    void GameMediaDatabase::addWeapon(const std::string& weaponName, WeaponMediaInfo&& weapon)
    {
        weaponMap.insert({toUpper(weaponName), std::move(weapon)});
    }

    const SoundClass defaultSoundClass = SoundClass();

    const SoundClass& GameMediaDatabase::getSoundClassOrDefault(const std::string& className) const
    {
        auto it = soundClassMap.find(className);
        if (it == soundClassMap.end())
        {
            return defaultSoundClass;
        }

        return it->second;
    }

    const SoundClass& GameMediaDatabase::getSoundClass(const std::string& className) const
    {
        auto it = soundClassMap.find(className);
        if (it == soundClassMap.end())
        {
            throw std::runtime_error("No sound class found with name " + className);
        }

        return it->second;
    }

    void GameMediaDatabase::addSoundClass(const std::string& className, SoundClass&& soundClass)
    {
        soundClassMap.insert({className, std::move(soundClass)});
    }

    const AudioService::SoundHandle& GameMediaDatabase::getSoundHandle(const std::string& sound) const
    {
        auto it = soundMap.find(sound);
        if (it == soundMap.end())
        {
            throw std::runtime_error("No sound found with name " + sound);
        }

        return it->second;
    }

    std::optional<AudioService::SoundHandle> GameMediaDatabase::tryGetSoundHandle(const std::string& sound) const
    {
        auto it = soundMap.find(sound);
        if (it == soundMap.end())
        {
            return std::nullopt;
        }

        return it->second;
    }

    void GameMediaDatabase::addSound(const std::string& soundName, const AudioService::SoundHandle& sound)
    {
        soundMap.insert({soundName, sound});
    }

    void GameMediaDatabase::addSelectionCollisionMesh(const std::string& objectName, std::shared_ptr<CollisionMesh> mesh)
    {
        selectionCollisionMeshesMap.insert({objectName, mesh});
    }

    std::optional<std::shared_ptr<CollisionMesh>> GameMediaDatabase::getSelectionCollisionMesh(const std::string& objectName) const
    {
        auto it = selectionCollisionMeshesMap.find(objectName);
        if (it == selectionCollisionMeshesMap.end())
        {
            return std::nullopt;
        }
        return it->second;
    }

    const FeatureMediaInfo& GameMediaDatabase::getFeature(FeatureDefinitionId featureId) const
    {
        return featureMap.get(featureId);
    }

    FeatureMediaInfo& GameMediaDatabase::getFeature(FeatureDefinitionId featureId)
    {
        return featureMap.get(featureId);
    }

    FeatureDefinitionId GameMediaDatabase::addFeature(FeatureMediaInfo&& feature)
    {
        return featureMap.insert(std::move(feature));
    }
}
