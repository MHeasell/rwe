#ifndef RWE_TDF_H
#define RWE_TDF_H

#include <rwe/tdf/SimpleTdfAdapter.h>

namespace rwe
{
    std::vector<TdfBlockEntry> parseTdf(ConstUtf8Iterator& begin, ConstUtf8Iterator& end);

    std::vector<TdfBlockEntry> parseTdfFromString(const std::string& input);
}

#endif
