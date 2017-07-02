#include <iostream>
#include <rwe/vfs/CompositeVirtualFileSystem.h>

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "Specify a search path and file to extract" << std::endl;
        return 1;
    }

    std::string searchPath(argv[1]);
    std::string file(argv[2]);

    auto vfs = rwe::constructVfs(searchPath);

    auto buffer = vfs->readFile(file);
    if (!buffer)
    {
        std::cerr << "File not found" << std::endl;
        return 1;
    }

    std::cout.write(buffer->data(), buffer->size());
    std::cout.flush();

    return 0;
}
