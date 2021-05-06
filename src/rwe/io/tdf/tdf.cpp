#include "tdf.h"

#include <rwe/io/tdf/ListTdfAdapter.h>
#include <rwe/io/tdf/SimpleTdfAdapter.h>
#include <rwe/io/tdf/NetSchemaTdfAdapter.h>

namespace rwe
{
    bool parseTdfHasNetworkSchema(const std::vector<char>& input)
    {
        TdfParser<std::vector<char>::const_iterator, bool> parser(new NetSchemaTdfAdapter);
        return parser.parse(input.begin(), input.end());
    }

    TdfBlock parseTdfFromString(const std::string& input)
    {
        TdfParser<ConstUtf8UncheckedIterator, TdfBlock> parser(new SimpleTdfAdapter);

        // TA files typically use legacy ISO-8859-1 encoding (latin1)
        // so fall back to that if the input isn't valid UTF8.
        if (!utf8::is_valid(input.begin(), input.end()))
        {
            auto convertedInput = latin1ToUtf8(input);
            return parser.parse(cUtf8UncheckedBegin(convertedInput), cUtf8UncheckedEnd(convertedInput));
        }

        return parser.parse(cUtf8UncheckedBegin(input), cUtf8UncheckedEnd(input));
    }

    std::vector<TdfBlock> parseListTdfFromString(const std::string& input)
    {
        TdfParser<ConstUtf8UncheckedIterator, std::vector<TdfBlock>> parser(new ListTdfAdapter);

        // TA files typically use legacy ISO-8859-1 encoding (latin1)
        // so fall back to that if the input isn't valid UTF8.
        if (!utf8::is_valid(input.begin(), input.end()))
        {
            auto convertedInput = latin1ToUtf8(input);
            return parser.parse(cUtf8UncheckedBegin(convertedInput), cUtf8UncheckedEnd(convertedInput));
        }

        return parser.parse(cUtf8UncheckedBegin(input), cUtf8UncheckedEnd(input));
    }

    TdfBlock parseTdfFromBytes(const std::vector<char>& bytes)
    {
        std::string tdfString(bytes.data(), bytes.size());
        return parseTdfFromString(tdfString);
    }

    std::vector<TdfBlock> parseListTdfFromBytes(const std::vector<char>& bytes)
    {
        std::string tdfString(bytes.data(), bytes.size());
        return parseListTdfFromString(tdfString);
    }
}
