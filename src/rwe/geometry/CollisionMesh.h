#ifndef RWE_COLLISIONMESH_H
#define RWE_COLLISIONMESH_H

#include <rwe/geometry/Ray3f.h>
#include <rwe/geometry/Triangle3f.h>

namespace rwe
{
    struct CollisionMesh
    {
        static CollisionMesh fromQuad(
            const Vector3f& a,
            const Vector3f& b,
            const Vector3f& c,
            const Vector3f& d);

        std::vector<Triangle3f> triangles;

        CollisionMesh() = default;
        explicit CollisionMesh(std::vector<Triangle3f>&& triangles);

        boost::optional<float> intersect(const Ray3f& ray) const;

        boost::optional<Vector3f> intersectLine(const Line3f& line) const;
    };
}

#endif
