#pragma once


#include <array>
#include <rwe/ColorPalette.h>
#include <rwe/math/Vector2f.h>
#include <rwe/math/Vector3f.h>
#include <rwe/render/TextureHandle.h>

namespace rwe
{
    struct Mesh
    {
        struct Vertex
        {
            Vector3f position;
            Vector2f textureCoord;

            Vertex() = default;
            Vertex(const Vector3f& position, const Vector2f& textureCoord);
        };

        struct Triangle
        {
            Vertex a;
            Vertex b;
            Vertex c;

            Triangle() = default;
            Triangle(const Vertex& a, const Vertex& b, const Vertex& c);
        };

        std::vector<Triangle> faces;
        std::vector<Triangle> teamFaces;
    };
}
