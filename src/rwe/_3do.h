#ifndef RWE_3DO_H
#define RWE_3DO_H

#include <string>
#include <vector>
#include <rwe/math/Vector3f.h>

namespace rwe
{
    static const unsigned int _3doMagicNumber = 1;

#pragma pack(1)
    struct _3doObject
    {
        unsigned int magicNumber;
        unsigned int numberOfVertices;
        unsigned int numberOfPrimitives;
        unsigned int selectionPrimitiveOffset;
        int xFromParent;
        int yFromParent;
        int zFromparent;
        unsigned int unknown1;
        unsigned int verticesOffset;
        unsigned int primitivesOffset;
        unsigned int siblingOffset;
        unsigned int firstChildOffset;
    };

    struct _3doVertex
    {
        int x;
        int y;
        int z;
    };

    struct _3doPrimitive
    {
        unsigned int colorIndex;
        unsigned int numberOfVertices;
        unsigned int unknown1;
        unsigned int verticesOffset;
        unsigned int textureNameOffset;
        unsigned int unknown2;
        unsigned int unknown3;
        unsigned int unknown4;
    };
#pragma pack()
    class _3do
    {

    };
}

#endif
