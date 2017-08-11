#ifndef RWE_TDF_H
#define RWE_TDF_H

#include <rwe/tdf/SimpleTdfAdapter.h>

namespace rwe
{
    std::vector<TdfBlockEntry> parseTdf(ConstUtf8Iterator& begin, ConstUtf8Iterator& end)
    {
        TdfParser<ConstUtf8Iterator, std::vector<TdfBlockEntry>> parser(new SimpleTdfAdapter);
        return parser.parse(begin, end);
    }
}

#endif
