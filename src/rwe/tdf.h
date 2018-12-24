#ifndef RWE_TDF_H
#define RWE_TDF_H

#include <rwe/rwe_string.h>
#include <rwe/tdf/TdfBlock.h>

namespace rwe
{
    TdfBlock parseTdf(ConstUtf8Iterator& begin, ConstUtf8Iterator& end);

    TdfBlock parseTdfFromString(const std::string& input);

    TdfBlock parseTdfFromBytes(const std::vector<char>& bytes);
}

#endif
