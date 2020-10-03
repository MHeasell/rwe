#pragma once

#include <functional>
#include <initializer_list>
#include <memory>
#include <optional>
#include <rwe/rwe_string.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace rwe
{
    template <typename T>
    std::optional<T> tdfTryParse(const std::string& value)
    {
        std::stringstream s(value);
        T i;
        s >> i;
        if (s.fail())
        {
            return std::nullopt;
        }

        return i;
    }

    template <>
    std::optional<std::string> tdfTryParse<std::string>(const std::string& value);

    template <>
    std::optional<bool> tdfTryParse<bool>(const std::string& value);

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

        std::optional<std::reference_wrapper<const TdfBlock>> findBlock(const std::string& name) const;
        std::optional<std::reference_wrapper<const std::string>> findValue(const std::string& name) const;

        bool operator==(const TdfBlock& rhs) const;

        bool operator!=(const TdfBlock& rhs) const;

        void insertOrAssignProperty(const std::string& name, const std::string& value);
        TdfBlock& insertOrAssignBlock(const std::string& name, const TdfBlock& block);
        TdfBlock& createBlock(const std::string& name);

        std::string expectString(const std::string& key) const;

        std::optional<int> extractInt(const std::string& key) const;

        std::optional<unsigned int> extractUint(const std::string& key) const;

        std::optional<float> extractFloat(const std::string& key) const;

        int expectInt(const std::string& key) const;

        unsigned int expectUint(const std::string& key) const;

        std::optional<bool> extractBool(const std::string& key) const;

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
            out = extract<T>(key).value_or(defaultValue);
        }

        template <typename T>
        std::optional<T> extract(const std::string& key) const
        {
            auto value = findValue(key);
            if (!value)
            {
                return std::nullopt;
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
        using ValueType = std::variant<TdfBlock, std::string>;
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
