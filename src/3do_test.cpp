#include <boost/optional/optional_io.hpp>
#include <rwe/_3do.h>
#include <iostream>
#include <fstream>
#include <vector>

void print3doObject(unsigned int indent, const std::vector<rwe::_3do::Object>& os)
{
    for (const auto& o : os)
    {
        std::string indentString(indent, ' ');
        std::cout << indentString << "name: " << o.name << std::endl;
        std::cout << indentString << "offset: " << "(" << o.x << ", " << o.y << ", " << o.z << ")" << std::endl;
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
        std::cerr << "Specify a 3do file to dump." << std::endl;
        return 1;
    }

    std::string filename(argv[1]);

    std::ifstream fh(filename, std::ios::binary);

    auto objects = rwe::parse3doObjects(fh, fh.tellg());
    print3doObject(0, objects);

    return 0;
}
