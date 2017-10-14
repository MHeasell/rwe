#ifndef RWE_UNITMESH_H
#define RWE_UNITMESH_H

#include <rwe/GraphicsContext.h>
#include <rwe/math/Vector3f.h>
#include <string>


namespace rwe
{
    struct UnitMesh
    {
        std::string name;
        Vector3f origin;
        std::shared_ptr<Mesh> mesh;
        std::vector<UnitMesh> children;

        void render(GraphicsContext& context) const;
    };
}

#endif
