#pragma once

#include <cassert>
#include <istream>

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
