#ifndef RWE_UNITDATABASE_H
#define RWE_UNITDATABASE_H

#include "AudioService.h"
#include "MovementClass.h"
#include "SoundClass.h"
#include "UnitFbi.h"
#include <rwe/Cob.h>

namespace rwe
{
    class UnitDatabase
    {
    private:
        std::unordered_map<std::string, UnitFbi> map;

        std::unordered_map<std::string, CobScript> cobMap;

        std::unordered_map<std::string, SoundClass> soundClassMap;

        std::unordered_map<std::string, MovementClass> movementClassMap;

        std::unordered_map<std::string, AudioService::SoundHandle> soundMap;

    public:
        const UnitFbi& getUnitInfo(const std::string& unitName) const;

        void addUnitInfo(const std::string& unitName, const UnitFbi& info);

        const CobScript& getUnitScript(const std::string& unitName) const;

        void addUnitScript(const std::string& unitName, CobScript&& cob);

        const SoundClass& getSoundClass(const std::string& className) const;

        void addSoundClass(const std::string& className, SoundClass&& soundClass);

        const MovementClass& getMovementClass(const std::string& className) const;

        void addMovementClass(const std::string& className, MovementClass&& movementClass);

        const AudioService::SoundHandle& getSoundHandle(const std::string sound) const;

        void addSound(const std::string& soundName, const AudioService::SoundHandle& sound);
    };
}

#endif
