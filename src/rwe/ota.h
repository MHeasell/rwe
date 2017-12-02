#ifndef RWE_OTA_H
#define RWE_OTA_H

#include <boost/optional.hpp>
#include <rwe/tdf/TdfBlock.h>
#include <string>
#include <vector>

namespace rwe
{
    class OtaParseException : public std::runtime_error
    {
    public:
        explicit OtaParseException(const std::string& __arg);
        explicit OtaParseException(const char* string);
    };

    struct OtaSpecial
    {
        std::string specialWhat;
        int xPos;
        int zPos;
    };

    struct OtaFeature
    {
        std::string featureName;
        int xPos;
        int zPos;
    };

    struct OtaSchema
    {
        std::string type;
        std::string aiProfile;
        int surfaceMetal;
        int mohoMetal;
        int humanMetal;
        int computerMetal;
        int humanEnergy;
        int computerEnergy;
        std::string meteorWeapon;
        int meteorRadius;
        float meteorDensity;
        int meteorDuration;
        int meteorInterval;

        std::vector<OtaFeature> features;
        std::vector<OtaSpecial> specials;
    };

    struct OtaRecord
    {
        std::string missionName;
        std::string missionDescription;
        std::string planet;
        std::string missionHint;
        std::string brief;
        std::string narration;
        std::string glamour;
        int lineOfSight;
        int mapping;
        int tidalStrength;
        int solarStrength;
        bool lavaWorld;
        int killMul;
        int timeMul;
        int minWindSpeed;
        int maxWindSpeed;
        int gravity;
        std::string numPlayers;
        std::string size;
        std::string memory;
        std::string useOnlyUnits;
        int schemaCount;

        std::vector<OtaSchema> schemas;
    };

    OtaSchema parseOtaSchema(const TdfBlock& tdf);

    OtaRecord parseOta(const TdfBlock& tdf);

    OtaRecord parseOtaGlobalHeader(const TdfBlock& tdf);

    OtaFeature parseOtaFeature(const TdfBlock& tdf);

    OtaSpecial parseOtaSpecial(const TdfBlock& tdf);
}

#endif
