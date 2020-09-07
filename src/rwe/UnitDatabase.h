#pragma once

#include <boost/functional/hash.hpp>
#include <memory>
#include <rwe/AudioService.h>
#include <rwe/Cob.h>
#include <rwe/MovementClass.h>
#include <rwe/SelectionMesh.h>
#include <rwe/ShaderMesh.h>
#include <rwe/SoundClass.h>
#include <rwe/UnitModelDefinition.h>
#include <rwe/WeaponTdf.h>
#include <rwe/fbi/UnitFbi.h>
#include <utility>

namespace rwe
{
    class UnitDatabase
    {
    public:
        using MovementClassIterator = typename std::unordered_map<std::string, MovementClass>::const_iterator;

    private:
        std::unordered_map<std::string, UnitFbi> map;

        std::unordered_map<std::string, CobScript> cobMap;

        std::unordered_map<std::string, WeaponTdf> weaponMap;

        std::unordered_map<std::string, SoundClass> soundClassMap;

        std::unordered_map<std::string, MovementClass> movementClassMap;

        std::unordered_map<std::string, AudioService::SoundHandle> soundMap;

        std::unordered_map<std::string, std::vector<std::vector<GuiEntry>>> builderGuisMap;

        std::unordered_map<std::pair<std::string, std::string>, std::shared_ptr<ShaderMesh>, boost::hash<std::pair<std::string, std::string>>> unitPieceMeshesMap;

        std::unordered_map<std::string, UnitModelDefinition> unitModelDefinitionsMap;

        std::unordered_map<std::string, std::shared_ptr<SelectionMesh>> selectionMeshesMap;

    public:
        bool hasUnitInfo(const std::string& unitName) const;

        const UnitFbi& getUnitInfo(const std::string& unitName) const;

        void addUnitInfo(const std::string& unitName, const UnitFbi& info);

        const CobScript& getUnitScript(const std::string& unitName) const;

        void addUnitScript(const std::string& unitName, CobScript&& cob);

        const WeaponTdf& getWeapon(const std::string& weaponName) const;

        std::optional<std::reference_wrapper<const WeaponTdf>> tryGetWeapon(const std::string& weaponName) const;

        void addWeapon(const std::string& name, WeaponTdf&& weapon);

        const SoundClass& getSoundClassOrDefault(const std::string& className) const;

        const SoundClass& getSoundClass(const std::string& className) const;

        void addSoundClass(const std::string& className, SoundClass&& soundClass);

        const MovementClass& getMovementClass(const std::string& className) const;

        void addMovementClass(const std::string& className, MovementClass&& movementClass);

        const AudioService::SoundHandle& getSoundHandle(const std::string& sound) const;

        std::optional<AudioService::SoundHandle> tryGetSoundHandle(const std::string& sound);

        void addSound(const std::string& soundName, const AudioService::SoundHandle& sound);

        std::optional<std::reference_wrapper<const std::vector<std::vector<GuiEntry>>>> tryGetBuilderGui(const std::string& unitName) const;

        void addBuilderGui(const std::string& unitName, std::vector<std::vector<GuiEntry>>&& gui);

        MovementClassIterator movementClassBegin() const;

        MovementClassIterator movementClassEnd() const;

        void addUnitPieceMesh(const std::string& unitName, const std::string& pieceName, std::shared_ptr<ShaderMesh> pieceMesh);

        std::optional<std::shared_ptr<ShaderMesh>> getUnitPieceMesh(const std::string& objectName, const std::string& pieceName) const;

        void addUnitModelDefinition(const std::string& objectName, UnitModelDefinition&& model);

        std::optional<std::reference_wrapper<const UnitModelDefinition>> getUnitModelDefinition(const std::string& objectName) const;

        std::optional<std::shared_ptr<SelectionMesh>> getSelectionMesh(const std::string& objectName) const;

        void addSelectionMesh(const std::string& objectName, std::shared_ptr<SelectionMesh> mesh);
    };
}
