#include "MeshDatabase.h"

namespace rwe
{
    void MeshDatabase::addSelectionMesh(const std::string& objectName, std::shared_ptr<GlMesh> mesh)
    {
        selectionMeshesMap.insert({objectName, mesh});
    }

    std::optional<std::shared_ptr<GlMesh>> MeshDatabase::getSelectionMesh(const std::string& objectName) const
    {
        auto it = selectionMeshesMap.find(objectName);
        if (it == selectionMeshesMap.end())
        {
            return std::nullopt;
        }
        return it->second;
    }

    void MeshDatabase::addUnitPieceMesh(const std::string& unitName, const std::string& pieceName, std::shared_ptr<ShaderMesh> pieceMesh)
    {
        unitPieceMeshesMap.insert({{unitName, pieceName}, pieceMesh});
    }

    std::optional<std::shared_ptr<ShaderMesh>> MeshDatabase::getUnitPieceMesh(const std::string& objectName, const std::string& pieceName) const
    {
        auto it = unitPieceMeshesMap.find({objectName, pieceName});
        if (it == unitPieceMeshesMap.end())
        {
            return std::nullopt;
        }
        return it->second;
    }

    void MeshDatabase::addSpriteSeries(const std::string& gafName, const std::string& animName, std::shared_ptr<SpriteSeries> sprite)
    {
        spritesMap.insert({{gafName, animName}, sprite});
    }

    std::optional<std::shared_ptr<SpriteSeries>> MeshDatabase::getSpriteSeries(const std::string& gaf, const std::string& anim) const
    {
        auto it = spritesMap.find({gaf, anim});
        if (it == spritesMap.end())
        {
            return std::nullopt;
        }
        return it->second;
    }

    const WeaponMediaInfo& MeshDatabase::getWeapon(const std::string& weaponName) const
    {
        auto it = weaponMap.find(toUpper(weaponName));
        if (it == weaponMap.end())
        {
            throw std::runtime_error("No weapon found with name " + weaponName);
        }

        return it->second;
    }

    std::optional<std::reference_wrapper<const WeaponMediaInfo>> MeshDatabase::tryGetWeapon(const std::string& weaponName) const
    {
        auto it = weaponMap.find(toUpper(weaponName));
        if (it == weaponMap.end())
        {
            return std::nullopt;
        }

        return it->second;
    }

    void MeshDatabase::addWeapon(const std::string& weaponName, WeaponMediaInfo&& weapon)
    {
        weaponMap.insert({toUpper(weaponName), std::move(weapon)});
    }

    const SoundClass defaultSoundClass = SoundClass();

    const SoundClass& MeshDatabase::getSoundClassOrDefault(const std::string& className) const
    {
        auto it = soundClassMap.find(className);
        if (it == soundClassMap.end())
        {
            return defaultSoundClass;
        }

        return it->second;
    }

    const SoundClass& MeshDatabase::getSoundClass(const std::string& className) const
    {
        auto it = soundClassMap.find(className);
        if (it == soundClassMap.end())
        {
            throw std::runtime_error("No sound class found with name " + className);
        }

        return it->second;
    }

    void MeshDatabase::addSoundClass(const std::string& className, SoundClass&& soundClass)
    {
        soundClassMap.insert({className, std::move(soundClass)});
    }

    const AudioService::SoundHandle& MeshDatabase::getSoundHandle(const std::string& sound) const
    {
        auto it = soundMap.find(sound);
        if (it == soundMap.end())
        {
            throw std::runtime_error("No sound found with name " + sound);
        }

        return it->second;
    }

    std::optional<AudioService::SoundHandle> MeshDatabase::tryGetSoundHandle(const std::string& sound) const
    {
        auto it = soundMap.find(sound);
        if (it == soundMap.end())
        {
            return std::nullopt;
        }

        return it->second;
    }

    void MeshDatabase::addSound(const std::string& soundName, const AudioService::SoundHandle& sound)
    {
        soundMap.insert({soundName, sound});
    }

    void MeshDatabase::addSelectionCollisionMesh(const std::string& objectName, std::shared_ptr<CollisionMesh> mesh)
    {
        selectionCollisionMeshesMap.insert({objectName, mesh});
    }

    std::optional<std::shared_ptr<CollisionMesh>> MeshDatabase::getSelectionCollisionMesh(const std::string& objectName) const
    {
        auto it = selectionCollisionMeshesMap.find(objectName);
        if (it == selectionCollisionMeshesMap.end())
        {
            return std::nullopt;
        }
        return it->second;
    }
}
