#include "tdf.h"

namespace rwe
{
    TdfValueException::TdfValueException(const std::string& message) : runtime_error(message)
    {
    }

    TdfValueException::TdfValueException(const char* message) : runtime_error(message)
    {
    }

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

    boost::optional<int> extractInt(const TdfBlock& block, const std::string& key)
    {
        auto value = block.findValue(key);
        if (!value)
        {
            return boost::none;
        }

        // convert the value to an integer
        try
        {
            return std::stoi(*value);
        }
        catch (const std::invalid_argument& e)
        {
            return boost::none;
        }
    }

    boost::optional<float> extractFloat(const TdfBlock& block, const std::string& key)
    {
        auto value = block.findValue(key);
        if (!value)
        {
            return boost::none;
        }

        // convert the value to an integer
        try
        {
            return std::stof(*value);
        }
        catch (const std::invalid_argument& e)
        {
            return boost::none;
        }
    }

    const std::string& expectString(const TdfBlock& block, const std::string& key)
    {
        auto v = block.findValue(key);
        if (!v)
        {
            throw TdfValueException("Failed to read string from key: " + key);
        }

        return *v;
    }

    int expectInt(const TdfBlock& block, const std::string& key)
    {
        auto v = extractInt(block, key);
        if (!v)
        {
            throw TdfValueException("Failed to read int from key: " + key);
        }

        return *v;
    }

    float expectFloat(const TdfBlock& block, const std::string& key)
    {
        auto v = extractFloat(block, key);
        if (!v)
        {
            throw TdfValueException("Failed to read int from key: " + key);
        }

        return *v;
    }

    boost::optional<bool> extractBool(const TdfBlock& block, const std::string& key)
    {
        auto value = block.findValue(key);
        if (!value)
        {
            return boost::none;
        }

        // convert the value to an integer
        int i;
        try
        {
            i = std::stoi(*value);
        }
        catch (const std::invalid_argument& e)
        {
            return boost::none;
        }

        return i != 0;
    }

    bool expectBool(const TdfBlock& block, const std::string& key)
    {
        auto v = extractBool(block, key);
        if (!v)
        {
            throw TdfValueException("Failed to read bool from key: " + key);
        }

        return *v;
    }
}
