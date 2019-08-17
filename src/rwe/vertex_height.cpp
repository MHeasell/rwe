#include "vertex_height.h"

namespace rwe
{
    bool compareVertexHeights(const _3do::Vertex& a, const _3do::Vertex& b)
    {
        return a.y < b.y;
    }
    _3do::Vertex findHighestVertex(const _3do::Object& obj)
    {
        auto it = std::max_element(obj.vertices.begin(), obj.vertices.end(), compareVertexHeights);
        assert(it != obj.vertices.end());
        auto highestVertex = *it;

        auto highestChildVertex = findHighestVertexOfList(obj.children);

        auto highest = highestChildVertex
            ? std::max(highestVertex, *highestChildVertex, compareVertexHeights)
            : highestVertex;

        return _3do::Vertex(highest.x + obj.x, highest.y + obj.y, highest.z + obj.z);
    }
    std::optional<_3do::Vertex> findHighestVertexOfList(const std::vector<_3do::Object>& os)
    {
        std::vector<_3do::Vertex> highestVertices;
        std::transform(os.begin(), os.end(), std::back_inserter(highestVertices), findHighestVertex);
        auto it = std::max_element(highestVertices.begin(), highestVertices.end(), compareVertexHeights);
        if (it == highestVertices.end())
        {
            return std::nullopt;
        }

        return *it;
    }
}
