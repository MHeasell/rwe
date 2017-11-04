#include "SoundClass.h"

namespace rwe
{
    SoundClass parseSoundClass(const TdfBlock& block)
    {
        SoundClass s;

        s.select1 = block.findValue("select1");
        s.ok1 = block.findValue("ok1");
        s.arrived1 = block.findValue("arrived1");
        s.cant1 = block.findValue("cant1");
        s.underAttack = block.findValue("underattack");
        s.count5 = block.findValue("count5");
        s.count4 = block.findValue("count4");
        s.count3 = block.findValue("count3");
        s.count2 = block.findValue("count2");
        s.count1 = block.findValue("count1");
        s.cancelDestruct = block.findValue("canceldestruct");

        return s;
    }

    std::vector<std::pair<std::string, SoundClass>> parseSoundTdf(const TdfBlock& root)
    {
        std::vector<std::pair<std::string, SoundClass>> vec;
        vec.reserve(root.entries.size());

        for (const auto& e : root.entries)
        {
            const auto& block = boost::get<TdfBlock>(*(e.value));
            vec.emplace_back(e.name, parseSoundClass(block));
        }

        return vec;
    }
}
