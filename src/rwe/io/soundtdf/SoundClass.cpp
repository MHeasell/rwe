#include "SoundClass.h"

namespace rwe
{
    SoundClass parseSoundClass(const TdfBlock& block)
    {
        SoundClass s;

        s.select1 = block.findValue("select1");
        s.unitComplete = block.findValue("unitcomplete");
        s.activate = block.findValue("activate");
        s.deactivate = block.findValue("deactivate");
        s.ok1 = block.findValue("ok1");
        s.arrived1 = block.findValue("arrived1");
        s.cant1 = block.findValue("cant1");
        s.underAttack = block.findValue("underattack");
        s.build = block.findValue("build");
        s.repair = block.findValue("repair");
        s.working = block.findValue("working");
        s.cloak = block.findValue("cloak");
        s.uncloak = block.findValue("uncloak");
        s.capture = block.findValue("capture");
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
        vec.reserve(root.blocks.size());

        for (const auto& e : root.blocks)
        {
            vec.emplace_back(e.first, parseSoundClass(*e.second));
        }

        return vec;
    }
}
