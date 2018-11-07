#include <fstream>
#include <iostream>
#include <rwe/_3do.h>
#include <rwe/fixed_point.h>
#include <rwe/optional_io.h>
#include <vector>
#include <cassert>

namespace rwe
{
    std::optional<_3do::Vertex> findHighestVertexOfList(const std::vector<_3do::Object>& os);

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

void print3doObject(unsigned int indent, const std::vector<rwe::_3do::Object>& os)
{
    for (const auto& o : os)
    {
        std::string indentString(indent, ' ');
        std::cout << indentString << "name: " << o.name << std::endl;
        std::cout << indentString << "offset: "
                  << "(" << rwe::fromFixedPoint(o.x) << ", " << rwe::fromFixedPoint(o.y) << ", " << rwe::fromFixedPoint(o.z) << ")" << std::endl;
        std::cout << indentString << "primitives: " << o.primitives.size() << std::endl;
        std::cout << indentString << "vertices: " << o.vertices.size() << std::endl;
        std::cout << indentString << "selection primitive: " << o.selectionPrimitiveIndex << std::endl;

        std::cout << std::endl;

        print3doObject(indent + 2, o.children);
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Specify a command" << std::endl;
        return 1;
    }

    std::string cmd(argv[1]);

    if (cmd == "dump")
    {
        if (argc < 3)
        {
            std::cerr << "Specify a 3do file to dump." << std::endl;
            return 1;
        }

        std::string filename(argv[2]);

        std::ifstream fh(filename, std::ios::binary);

        auto objects = rwe::parse3doObjects(fh, fh.tellg());
        print3doObject(0, objects);
        return 0;
    }

    if (cmd == "height")
    {
        if (argc < 3)
        {
            std::cerr << "Specify a 3do file." << std::endl;
            return 1;
        }

        std::string filename(argv[2]);

        std::ifstream fh(filename, std::ios::binary);

        auto objects = rwe::parse3doObjects(fh, fh.tellg());
        auto highestVertex = rwe::findHighestVertexOfList(objects);
        assert(highestVertex);
        std::cout << highestVertex->y << std::endl;
        return 0;
    }

    std::cerr << "Unrecognised command" << std::endl;
    return 1;
}
