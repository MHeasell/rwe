#ifndef RWE_IO_UTILS_H
#define RWE_IO_UTILS_H

#include <istream>
#include <cassert>

namespace rwe
{
    template <typename T>
    T readRaw(std::istream& stream)
    {
        T val;
        stream.read(reinterpret_cast<char*>(&val), sizeof(T));
        assert(!stream.fail());
        return val;
    }

    std::string readNullTerminatedString(std::istream& stream);

}

#endif
