#pragma once

#include <boost/functional/hash.hpp>
#include <memory>
#include <rwe/GlMesh.h>
#include <rwe/ShaderMesh.h>
#include <utility>

namespace rwe
{
    class MeshDatabase
    {
    private:
        std::unordered_map<std::pair<std::string, std::string>, std::shared_ptr<ShaderMesh>, boost::hash<std::pair<std::string, std::string>>> unitPieceMeshesMap;
        std::unordered_map<std::string, std::shared_ptr<GlMesh>> selectionMeshesMap;

    public:
        void addUnitPieceMesh(const std::string& unitName, const std::string& pieceName, std::shared_ptr<ShaderMesh> pieceMesh);

        std::optional<std::shared_ptr<ShaderMesh>> getUnitPieceMesh(const std::string& objectName, const std::string& pieceName) const;

        std::optional<std::shared_ptr<GlMesh>> getSelectionMesh(const std::string& objectName) const;

        void addSelectionMesh(const std::string& objectName, std::shared_ptr<GlMesh> mesh);
    };
}
