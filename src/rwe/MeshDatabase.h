#pragma once

#include <boost/algorithm/string/predicate.hpp>
#include <boost/functional/hash.hpp>
#include <memory>
#include <rwe/AudioService.h>
#include <rwe/FeatureMediaInfo.h>
#include <rwe/SoundClass.h>
#include <rwe/WeaponMediaInfo.h>
#include <rwe/geometry/CollisionMesh.h>
#include <rwe/render/GlMesh.h>
#include <rwe/render/ShaderMesh.h>
#include <rwe/render/SpriteSeries.h>
#include <rwe/rwe_string.h>
#include <utility>

namespace rwe
{
    class MeshDatabase
    {
    private:
        struct CaseInsensitiveHash
        {
            std::hash<std::string> hash;

            std::size_t operator()(const std::string& key) const
            {
                return hash(toUpper(key));
            }
        };

        struct CaseInsensitiveEquals
        {
            bool operator()(const std::string& a, const std::string& b) const
            {
                return toUpper(a) == toUpper(b);
            }
        };

        struct CaseInsensitivePairHash
        {
            std::size_t operator()(const std::pair<std::string, std::string>& key) const
            {
                std::size_t seed = 0;
                boost::hash_combine(seed, toUpper(key.first));
                boost::hash_combine(seed, toUpper(key.second));
                return seed;
            }
        };

        struct CaseInsensitivePairEquals
        {
            bool operator()(const std::pair<std::string, std::string>& a, const std::pair<std::string, std::string>& b) const
            {
                return toUpper(a.first) == toUpper(b.first) && toUpper(a.second) == toUpper(b.second);
            }
        };

    private:
        std::unordered_map<std::pair<std::string, std::string>, std::shared_ptr<ShaderMesh>, CaseInsensitivePairHash, CaseInsensitivePairEquals> unitPieceMeshesMap;
        std::unordered_map<std::string, std::shared_ptr<GlMesh>, CaseInsensitiveHash, CaseInsensitiveEquals> selectionMeshesMap;

        std::unordered_map<std::pair<std::string, std::string>, std::shared_ptr<SpriteSeries>, CaseInsensitivePairHash, CaseInsensitivePairEquals> spritesMap;

        std::unordered_map<std::string, WeaponMediaInfo> weaponMap;

        std::unordered_map<std::string, SoundClass> soundClassMap;

        std::unordered_map<std::string, AudioService::SoundHandle> soundMap;

        std::unordered_map<std::string, std::shared_ptr<CollisionMesh>> selectionCollisionMeshesMap;

        std::unordered_map<std::string, FeatureMediaInfo> featureMap;

    public:
        void addUnitPieceMesh(const std::string& unitName, const std::string& pieceName, std::shared_ptr<ShaderMesh> pieceMesh);

        std::optional<std::shared_ptr<ShaderMesh>> getUnitPieceMesh(const std::string& objectName, const std::string& pieceName) const;

        std::optional<std::shared_ptr<GlMesh>> getSelectionMesh(const std::string& objectName) const;

        void addSelectionMesh(const std::string& objectName, std::shared_ptr<GlMesh> mesh);

        void addSpriteSeries(const std::string& gafName, const std::string& animName, std::shared_ptr<SpriteSeries> sprite);

        std::optional<std::shared_ptr<SpriteSeries>> getSpriteSeries(const std::string& gaf, const std::string& anim) const;

        const WeaponMediaInfo& getWeapon(const std::string& weaponName) const;

        std::optional<std::reference_wrapper<const WeaponMediaInfo>> tryGetWeapon(const std::string& weaponName) const;

        void addWeapon(const std::string& name, WeaponMediaInfo&& weapon);

        const SoundClass& getSoundClassOrDefault(const std::string& className) const;

        const SoundClass& getSoundClass(const std::string& className) const;

        void addSoundClass(const std::string& className, SoundClass&& soundClass);

        const AudioService::SoundHandle& getSoundHandle(const std::string& sound) const;

        std::optional<AudioService::SoundHandle> tryGetSoundHandle(const std::string& sound) const;

        void addSound(const std::string& soundName, const AudioService::SoundHandle& sound);

        void addSelectionCollisionMesh(const std::string& objectName, std::shared_ptr<CollisionMesh> mesh);

        std::optional<std::shared_ptr<CollisionMesh>> getSelectionCollisionMesh(const std::string& objectName) const;

        const FeatureMediaInfo& getFeature(const std::string& FeatureName) const;

        void addFeature(const std::string& name, FeatureMediaInfo&& feature);
    };
}
