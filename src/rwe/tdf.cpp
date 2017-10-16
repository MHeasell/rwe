#include "tdf.h"

#include <rwe/tdf/SimpleTdfAdapter.h>

namespace rwe
{
    TdfBlock parseTdf(ConstUtf8Iterator& begin, ConstUtf8Iterator& end)
    {
        TdfParser<ConstUtf8Iterator, TdfBlock> parser(new SimpleTdfAdapter);
        return parser.parse(begin, end);
    }

    TdfBlock parseTdfFromString(const std::string& input)
    {
        TdfParser<ConstUtf8Iterator, TdfBlock> parser(new SimpleTdfAdapter);
        return parser.parse(cUtf8Begin(input), cUtf8End(input));
    }
}
