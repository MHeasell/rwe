#include "_3do.h"
#include <rwe/io/io_util.h>

namespace rwe
{
    std::vector<_3do::Object> parse3doObjects(std::istream& stream, std::istream::pos_type offset)
    {
        std::vector<_3do::Object> outputObjects;

        do
        {
            stream.seekg(offset);
            auto object = readRaw<_3doObject>(stream);

            assert(object.magicNumber == _3doMagicNumber);

            _3do::Object outputObject;

            outputObject.x = object.xFromParent;
            outputObject.y = object.yFromParent;
            outputObject.z = object.zFromparent;

            if (object.selectionPrimitiveOffset == -1)
            {
                outputObject.selectionPrimitiveIndex = std::nullopt;
            }
            else
            {
                outputObject.selectionPrimitiveIndex = object.selectionPrimitiveOffset;
            }

            stream.seekg(object.nameOffset);
            outputObject.name = readNullTerminatedString(stream);

            stream.seekg(object.verticesOffset);
            for (unsigned int i = 0; i < object.numberOfVertices; ++i)
            {
                auto v = readRaw<_3doVertex>(stream);
                outputObject.vertices.push_back(_3do::Vertex{v.x, v.y, v.z});
            }

            stream.seekg(object.primitivesOffset);
            std::vector<_3doPrimitive> ps;
            ps.reserve(object.numberOfPrimitives);
            for (unsigned int i = 0; i < object.numberOfPrimitives; ++i)
            {
                ps.push_back(readRaw<_3doPrimitive>(stream));
            }

            for (const auto& p : ps)
            {
                _3do::Primitive outP;

                if (p.isColored)
                {
                    outP.colorIndex = p.colorIndex;
                }

                if (p.textureNameOffset != 0)
                {
                    stream.seekg(p.textureNameOffset);
                    outP.textureName = readNullTerminatedString(stream);
                }
                else
                {
                    outP.textureName = std::nullopt;
                }

                stream.seekg(p.verticesOffset);
                for (unsigned int j = 0; j < p.numberOfVertices; ++j)
                {
                    outP.vertices.push_back(readRaw<uint16_t>(stream));
                }

                outputObject.primitives.push_back(std::move(outP));
            }

            if (object.firstChildOffset != 0)
            {
                auto children = parse3doObjects(stream, object.firstChildOffset);
                std::move(children.begin(), children.end(), std::back_inserter(outputObject.children));
            }

            outputObjects.push_back(std::move(outputObject));
            offset = std::streampos(object.siblingOffset);
        } while (offset != std::streampos(0));

        return outputObjects;
    }
}
