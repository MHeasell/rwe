#ifndef RWE_SOUNDCLASS_H
#define RWE_SOUNDCLASS_H

#include <optional>
#include <rwe/tdf/TdfBlock.h>
#include <string>

namespace rwe
{
    struct SoundClass
    {
        std::optional<std::string> select1;
        std::optional<std::string> unitComplete;
        std::optional<std::string> activate;
        std::optional<std::string> deactivate;
        std::optional<std::string> ok1;
        std::optional<std::string> arrived1;
        std::optional<std::string> cant1;
        std::optional<std::string> underAttack;
        std::optional<std::string> build;
        std::optional<std::string> repair;
        std::optional<std::string> working;
        std::optional<std::string> cloak;
        std::optional<std::string> uncloak;
        std::optional<std::string> capture;
        std::optional<std::string> count5;
        std::optional<std::string> count4;
        std::optional<std::string> count3;
        std::optional<std::string> count2;
        std::optional<std::string> count1;
        std::optional<std::string> count0;
        std::optional<std::string> cancelDestruct;
    };

    SoundClass parseSoundClass(const TdfBlock& block);

    std::vector<std::pair<std::string, SoundClass>> parseSoundTdf(const TdfBlock& root);
}

#endif
