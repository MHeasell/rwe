#ifndef RWE_IO_UTILS_H
#define RWE_IO_UTILS_H

#include <istream>

namespace rwe
{
    template <typename T>
    T readRaw(std::istream& stream)
    {
        T val;
        stream.read(reinterpret_cast<char*>(&val), sizeof(T));
        return val;
    }
}

#endif
