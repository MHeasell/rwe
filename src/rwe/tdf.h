#ifndef RWE_TDF_H
#define RWE_TDF_H

#include <rwe/tdf/SimpleTdfAdapter.h>

namespace rwe
{
    TdfBlock parseTdf(ConstUtf8Iterator& begin, ConstUtf8Iterator& end);

    TdfBlock parseTdfFromString(const std::string& input);
}

#endif
