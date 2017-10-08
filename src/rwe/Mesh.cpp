#include "Mesh.h"

namespace rwe
{

    Mesh::Triangle::Triangle(const Mesh::Vertex& a, const Mesh::Vertex& b, const Mesh::Vertex& c)
        : a(a), b(b), c(c)
    {
    }

    Mesh::Vertex::Vertex(const Vector3f& position, const Vector2f& textureCoord)
        : position(position),
          textureCoord(textureCoord)
    {
    }
}
