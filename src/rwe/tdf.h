#ifndef RWE_TDF_H
#define RWE_TDF_H

#include <rwe/tdf/SimpleTdfAdapter.h>

namespace rwe
{
    class TdfValueException : public std::runtime_error
    {
    public:
        explicit TdfValueException(const char* message);
        explicit TdfValueException(const std::string& message);
    };

    TdfBlock parseTdf(ConstUtf8Iterator& begin, ConstUtf8Iterator& end);

    TdfBlock parseTdfFromString(const std::string& input);

    const std::string& expectString(const TdfBlock& block, const std::string& key);

    boost::optional<int> extractInt(const TdfBlock& block, const std::string& key);

    boost::optional<unsigned int> extractUint(const TdfBlock& block, const std::string& key);

    boost::optional<float> extractFloat(const TdfBlock& block, const std::string& key);

    int expectInt(const TdfBlock& block, const std::string& key);

    boost::optional<bool> extractBool(const TdfBlock& block, const std::string& key);

    bool expectBool(const TdfBlock& block, const std::string& key);

    float expectFloat(const TdfBlock& block, const std::string& key);
}

#endif
