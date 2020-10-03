#include "Cob.h"
#include <rwe/io_utils.h>

namespace rwe
{
    CobScript parseCob(std::istream& stream)
    {
        auto header = readRaw<CobHeader>(stream);

        CobScript script;
        script.staticVariableCount = header.staticVariableCount;

        // read in the instructions
        script.instructions.resize(header.codeLength);
        stream.seekg(header.offsetToScriptCode);
        for (unsigned int i = 0; i < header.codeLength; ++i)
        {
            script.instructions[i] = readRaw<uint32_t>(stream);
        }


        // read in function addresses
        script.functions.resize(header.numberOfScripts);
        stream.seekg(header.offsetToScriptCodeIndexArray);
        for (unsigned int i = 0; i < header.numberOfScripts; ++i)
        {
            script.functions[i].address = readRaw<uint32_t>(stream);
        }

        // read in function names
        stream.seekg(header.offsetToScriptNameOffsetArray);
        for (unsigned int i = 0; i < header.numberOfScripts; ++i)
        {
            auto nameOffset = readRaw<uint32_t>(stream);
            auto loc = stream.tellg();
            stream.seekg(nameOffset);
            script.functions[i].name = readNullTerminatedString(stream);
            stream.seekg(loc);
        }

        // read in piece names
        script.pieces.resize(header.numberOfPieces);
        stream.seekg(header.offsetToPieceNameOffsetArray);
        for (unsigned int i = 0; i < header.numberOfPieces; ++i)
        {
            auto nameOffset = readRaw<uint32_t>(stream);
            auto loc = stream.tellg();
            stream.seekg(nameOffset);
            script.pieces[i] = readNullTerminatedString(stream);
            stream.seekg(loc);
        }

        return script;
    }
}
