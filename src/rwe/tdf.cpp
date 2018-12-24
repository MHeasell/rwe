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

        // TA files typically use legacy ISO-8859-1 encoding (latin1)
        // so fall back to that if the input isn't valid UTF8.
        if (!utf8::is_valid(input.begin(), input.end()))
        {
            auto convertedInput = latin1ToUtf8(input);
            return parser.parse(cUtf8Begin(convertedInput), cUtf8End(convertedInput));
        }

        return parser.parse(cUtf8Begin(input), cUtf8End(input));
    }

    TdfBlock parseTdfFromBytes(const std::vector<char>& bytes)
    {
        std::string tdfString(bytes.data(), bytes.size());
        return parseTdfFromString(tdfString);
    }
}
