#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <rwe/fixed_point.h>
#include <rwe/io/_3do/_3do.h>
#include <rwe/optional_io.h>
#include <rwe/vertex_height.h>
#include <vector>

void print3doObject(int indent, const std::vector<rwe::_3do::Object>& os)
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
