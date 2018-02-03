#ifndef RWE_TDFBLOCK_H
#define RWE_TDFBLOCK_H

#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <initializer_list>
#include <memory>
#include <rwe/rwe_string.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace rwe
{
    template <typename T>
    boost::optional<T> tdfTryParse(const std::string& value);

    template <>
    boost::optional<int> tdfTryParse<int>(const std::string& value);

    template <>
    boost::optional<unsigned int> tdfTryParse<unsigned int>(const std::string& value);

    template <>
    boost::optional<float> tdfTryParse<float>(const std::string& value);

    template <>
    boost::optional<std::string> tdfTryParse<std::string>(const std::string& value);

    template <>
    boost::optional<bool> tdfTryParse<bool>(const std::string& value);

    class TdfValueException : public std::runtime_error
    {
    public:
        explicit TdfValueException(const char* message);
        explicit TdfValueException(const std::string& message);
    };

    struct TdfPropertyValue;

    struct CaseInsensitiveHash
    {
        std::hash<std::string> hash;

        std::size_t operator()(const std::string& key) const
        {
            return hash(toUpper(key));
        }
    };

    struct CaseInsensitiveEquals
    {
        bool operator()(const std::string& a, const std::string& b) const
        {
            return toUpper(a) == toUpper(b);
        }
    };

    struct TdfBlock
    {
        using PropertyMap = std::unordered_map<std::string, std::string, CaseInsensitiveHash, CaseInsensitiveEquals>;
        using BlockMap = std::unordered_map<std::string, std::unique_ptr<TdfBlock>, CaseInsensitiveHash, CaseInsensitiveEquals>;

        PropertyMap properties;
        BlockMap blocks;

        TdfBlock() = default;
        explicit TdfBlock(PropertyMap&& entries);
        TdfBlock(const TdfBlock& other);
        TdfBlock& operator=(const TdfBlock& other);

        boost::optional<const TdfBlock&> findBlock(const std::string& name) const;
        boost::optional<const std::string&> findValue(const std::string& name) const;

        bool operator==(const TdfBlock& rhs) const;

        bool operator!=(const TdfBlock& rhs) const;

        void insertOrAssignProperty(const std::string& name, const std::string& value);
        TdfBlock& insertOrAssignBlock(const std::string& name, const TdfBlock& block);
        TdfBlock& createBlock(const std::string& name);

        std::string expectString(const std::string& key) const;

        boost::optional<int> extractInt(const std::string& key) const;

        boost::optional<unsigned int> extractUint(const std::string& key) const;

        boost::optional<float> extractFloat(const std::string& key) const;

        int expectInt(const std::string& key) const;

        unsigned int expectUint(const std::string& key) const;

        boost::optional<bool> extractBool(const std::string& key) const;

        bool expectBool(const std::string& key) const;

        float expectFloat(const std::string& key) const;

        template <typename T>
        void read(const std::string& key, T& out) const
        {
            out = expect<T>(key);
        }

        template <typename T>
        void readOrDefault(const std::string& key, T& out, const T& defaultValue = T()) const
        {
            out = extract<T>(key).get_value_or(defaultValue);
        }

        template <typename T>
        boost::optional<T> extract(const std::string& key) const
        {
            auto value = findValue(key);
            if (!value)
            {
                return boost::none;
            }

            return tdfTryParse<T>(*value);
        }

        template <typename T>
        T expect(const std::string& key) const
        {
            auto v = extract<T>(key);
            if (!v)
            {
                throw TdfValueException("Failed to read from key: " + key);
            }

            return *v;
        }
    };

    struct TdfPropertyValue
    {
        using ValueType = boost::variant<TdfBlock, std::string>;
        ValueType value;

        TdfPropertyValue(const std::string& s) : value(s) {}
        TdfPropertyValue(std::string&& s) : value(std::move(s)) {}

        TdfPropertyValue(const TdfBlock& s) : value(s) {}
        TdfPropertyValue(TdfBlock&& s) : value(std::move(s)) {}

        TdfPropertyValue(const ValueType& s) : value(s) {}
        TdfPropertyValue(ValueType&& s) : value(std::move(s)) {}

        bool operator==(const TdfPropertyValue& rhs) const;

        bool operator!=(const TdfPropertyValue& rhs) const
        {
            return !(rhs == *this);
        }
    };

    TdfBlock makeTdfBlock(std::vector<std::pair<std::string, TdfPropertyValue::ValueType>> list);
}

#endif
