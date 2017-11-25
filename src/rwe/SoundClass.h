#ifndef RWE_SOUNDCLASS_H
#define RWE_SOUNDCLASS_H

#include <boost/optional.hpp>
#include <rwe/tdf/TdfBlock.h>
#include <string>

namespace rwe
{
    struct SoundClass
    {
        boost::optional<std::string> select1;
        boost::optional<std::string> ok1;
        boost::optional<std::string> arrived1;
        boost::optional<std::string> cant1;
        boost::optional<std::string> underAttack;
        boost::optional<std::string> count5;
        boost::optional<std::string> count4;
        boost::optional<std::string> count3;
        boost::optional<std::string> count2;
        boost::optional<std::string> count1;
        boost::optional<std::string> count0;
        boost::optional<std::string> cancelDestruct;
    };

    SoundClass parseSoundClass(const TdfBlock& block);

    std::vector<std::pair<std::string, SoundClass>> parseSoundTdf(const TdfBlock& root);
}

#endif
