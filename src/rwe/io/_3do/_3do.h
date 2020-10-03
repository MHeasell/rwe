#pragma once

#include <istream>
#include <optional>
#include <string>
#include <vector>

namespace rwe
{
    static const unsigned int _3doMagicNumber = 1;

#pragma pack(1)
    struct _3doObject
    {
        uint32_t magicNumber;
        uint32_t numberOfVertices;
        uint32_t numberOfPrimitives;
        int32_t selectionPrimitiveOffset;
        int32_t xFromParent;
        int32_t yFromParent;
        int32_t zFromparent;
        uint32_t nameOffset;
        uint32_t unknown1;
        uint32_t verticesOffset;
        uint32_t primitivesOffset;
        uint32_t siblingOffset;
        uint32_t firstChildOffset;
    };

    struct _3doVertex
    {
        int32_t x;
        int32_t y;
        int32_t z;
    };

    struct _3doPrimitive
    {
        uint32_t colorIndex;
        uint32_t numberOfVertices;
        uint32_t unknown1;
        uint32_t verticesOffset;
        uint32_t textureNameOffset;
        uint32_t unknown2;
        uint32_t unknown3;
        uint32_t isColored;
    };
#pragma pack()
    struct _3do
    {
        struct Vertex
        {
            int x;
            int y;
            int z;

            Vertex() = default;
            Vertex(int x, int y, int z) : x(x), y(y), z(z) {}
        };

        struct Primitive
        {
            std::optional<unsigned int> colorIndex;
            std::vector<unsigned int> vertices;
            std::optional<std::string> textureName;
        };

        struct Object
        {
            int x;
            int y;
            int z;
            std::vector<Vertex> vertices;
            std::vector<Primitive> primitives;
            std::vector<Object> children;
            std::string name;
            std::optional<unsigned int> selectionPrimitiveIndex;
        };
    };

    std::vector<_3do::Object> parse3doObjects(std::istream& stream, std::istream::pos_type offset);
}
