#include "tdf.h"

namespace rwe
{
    std::vector<TdfBlockEntry> parseTdf(ConstUtf8Iterator& begin, ConstUtf8Iterator& end)
    {
        TdfParser<ConstUtf8Iterator, std::vector<TdfBlockEntry>> parser(new SimpleTdfAdapter);
        return parser.parse(begin, end);
    }

    std::vector<TdfBlockEntry> parseTdfFromString(const std::string& input)
    {
        TdfParser<ConstUtf8Iterator, std::vector<TdfBlockEntry>> parser(new SimpleTdfAdapter);
        return parser.parse(cUtf8Begin(input), cUtf8End(input));
    }
}
