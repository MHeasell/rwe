#include <iostream>
#include <string>
#include <rwe/_3do.h>
#include <rwe/fixed_point.h>
#include <rwe/optional_io.h>
#include <vector>
#include <rwe/rwe_string.h>
#include <rwe/vfs/CompositeVirtualFileSystem.h>

int main(int argc, char* argv[])
{
    rwe::CompositeVirtualFileSystem vfs;

    for (std::string line; std::getline(std::cin, line);)
    {
        auto tokens = rwe::utf8Split(line, '|');
        if (tokens.empty())
        {
            std::cout << "Failed to tokenize command" << std::endl;
            return 1;
        }

        auto& command = tokens[0];
        if (command == "clear-data-paths")
        {
            vfs.clear();
            std::cout << "OK" << std::endl;
        }
        else if (command == "add-data-path")
        {
            if (tokens.size() != 2)
            {
                std::cout << "Expected two operands, really got " << tokens.size() << std::endl;
                return 1;
            }

            rwe::addToVfs(vfs, tokens[1]);
            std::cout << "OK" << std::endl;
        }
        else if (command == "map-info")
        {
            if (tokens.size() != 2)
            {
                std::cout << "Expected two operands, really got " << tokens.size() << std::endl;
                return 1;
            }

            auto data = vfs.readFile("maps/" + tokens[1] + ".ota");
            std::cout.write(data->data(), data->size());
        }
        else
        {
            std::cout << "Unrecognised command" << std::endl;
            return 1;
        }
    }

    return 0;
}
