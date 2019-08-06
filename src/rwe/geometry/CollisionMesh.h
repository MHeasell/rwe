#pragma once

#include <rwe/geometry/Line3f.h>
#include <rwe/geometry/Ray3f.h>
#include <rwe/geometry/Triangle3f.h>
#include <rwe/math/Vector3f.h>
#include <vector>

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

        std::optional<float> intersect(const Ray3f& ray) const;

        std::optional<Vector3f> intersectLine(const Line3f& line) const;
    };
}
