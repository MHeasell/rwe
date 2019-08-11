#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace rwe
{
#pragma pack(1)
    struct CobHeader
    {
        uint32_t magicNumber;
        uint32_t numberOfScripts;
        uint32_t numberOfPieces;
        uint32_t codeLength;
        uint32_t staticVariableCount;
        uint32_t unknown0;
        uint32_t offsetToScriptCodeIndexArray;
        uint32_t offsetToScriptNameOffsetArray;
        uint32_t offsetToPieceNameOffsetArray;
        uint32_t offsetToScriptCode;
        uint32_t unknown1;
    };
#pragma pack()

    struct CobFunctionInfo
    {
        std::string name;
        unsigned int address;
    };

    struct CobScript
    {
        std::vector<uint32_t> instructions;
        std::vector<std::string> pieces;
        std::vector<CobFunctionInfo> functions;
        unsigned int staticVariableCount;
    };

    CobScript parseCob(std::istream& stream);
}
