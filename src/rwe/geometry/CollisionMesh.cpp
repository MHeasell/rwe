#include "CollisionMesh.h"
#include <cmath>

namespace rwe
{
    boost::optional<float> CollisionMesh::intersect(const Ray3f& ray) const
    {
        auto distance = std::numeric_limits<float>::infinity();

        for (const auto& t : triangles)
        {
            auto v = t.intersect(ray);
            if (v && *v < distance)
            {
                distance = *v;
            }
        }

        if (std::isinf(distance))
        {
            return boost::none;
        }

        return distance;
    }

    boost::optional<Vector3f> CollisionMesh::intersectLine(const Line3f& line) const
    {
        boost::optional<Vector3f> winner;

        for (const auto& t : triangles)
        {
            auto v = t.intersectLine(line);
            winner = closestTo(line.start, v, winner);
        }

        return winner;
    }

    CollisionMesh CollisionMesh::fromQuad(const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& d)
    {
        Triangle3f t1(a, b, c);
        Triangle3f t2(a, d, c);
        return CollisionMesh({t1, t2});
    }

    CollisionMesh::CollisionMesh(std::vector<Triangle3f>&& triangles) : triangles(triangles)
    {
    }
}
