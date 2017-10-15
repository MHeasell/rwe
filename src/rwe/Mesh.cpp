#include "Mesh.h"

namespace rwe
{

    Mesh::Triangle::Triangle(const Mesh::Vertex& a, const Mesh::Vertex& b, const Mesh::Vertex& c)
        : a(a), b(b), c(c), color(255, 255, 255)
    {
    }

    Mesh::Triangle::Triangle(const Mesh::Vertex& a, const Mesh::Vertex& b, const Mesh::Vertex& c, const Color& color)
        : a(a), b(b), c(c), color(color)
    {
    }

    Mesh::Vertex::Vertex(const Vector3f& position, const Vector2f& textureCoord)
        : position(position),
          textureCoord(textureCoord)
    {
    }
}
