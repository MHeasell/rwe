#include "TdfBlock.h"

#include <rwe/rwe_string.h>
#include <sstream>

namespace rwe
{
    template <>
    std::optional<std::string> tdfTryParse(const std::string& value)
    {
        return value;
    }

    template <>
    std::optional<bool> tdfTryParse(const std::string& value)
    {
        auto val = tdfTryParse<int>(value);
        if (!val)
        {
            return std::nullopt;
        }

        return val != 0;
    }

    class EqualityVisitor : public boost::static_visitor<bool>
    {
    private:
        const TdfPropertyValue::ValueType* other;

    public:
        explicit EqualityVisitor(const TdfPropertyValue::ValueType* other) : other(other) {}
        bool operator()(const std::string& s) const
        {
            auto rhs = boost::get<std::string>(other);
            if (!rhs)
            {
                return false;
            }

            return s == *rhs;
        }

        bool operator()(const TdfBlock& b) const
        {
            auto rhs = boost::get<TdfBlock>(other);
            if (!rhs)
            {
                return false;
            }

            return b == *rhs;
        }
    };

    TdfValueException::TdfValueException(const std::string& message) : runtime_error(message)
    {
    }

    TdfValueException::TdfValueException(const char* message) : runtime_error(message)
    {
    }

    bool TdfBlock::operator==(const TdfBlock& rhs) const
    {
        if (properties != rhs.properties)
        {
            return false;
        }

        if (blocks.size() != rhs.blocks.size())
        {
            return false;
        }

        for (const auto& kv : blocks)
        {
            auto it = rhs.blocks.find(kv.first);
            if (it == rhs.blocks.end())
            {
                return false;
            }

            if (*kv.second != *it->second)
            {
                return false;
            }
        }

        return true;
    }

    bool TdfBlock::operator!=(const TdfBlock& rhs) const
    {
        return !(rhs == *this);
    }

    std::optional<std::reference_wrapper<const TdfBlock>> TdfBlock::findBlock(const std::string& name) const
    {
        // find the key in the block
        auto it = blocks.find(name);
        if (it == blocks.end())
        {
            return std::nullopt;
        }

        return *it->second;
    }

    std::optional<std::reference_wrapper<const std::string>> TdfBlock::findValue(const std::string& name) const
    {
        // find the key in the block
        auto it = properties.find(name);
        if (it == properties.end())
        {
            return std::nullopt;
        }

        return it->second;
    }

    std::optional<int> TdfBlock::extractInt(const std::string& key) const
    {
        return extract<int>(key);
    }

    std::optional<unsigned int> TdfBlock::extractUint(const std::string& key) const
    {
        return extract<unsigned int>(key);
    }

    std::optional<float> TdfBlock::extractFloat(const std::string& key) const
    {
        return extract<float>(key);
    }

    std::string TdfBlock::expectString(const std::string& key) const
    {
        return expect<std::string>(key);
    }

    int TdfBlock::expectInt(const std::string& key) const
    {
        return expect<int>(key);
    }

    float TdfBlock::expectFloat(const std::string& key) const
    {
        return expect<float>(key);
    }

    std::optional<bool> TdfBlock::extractBool(const std::string& key) const
    {
        return extract<bool>(key);
    }

    bool TdfBlock::expectBool(const std::string& key) const
    {
        return expect<bool>(key);
    }

    unsigned int TdfBlock::expectUint(const std::string& key) const
    {
        return expect<unsigned int>(key);
    }

    TdfBlock::BlockMap copyEntries(const TdfBlock::BlockMap& entries)
    {
        TdfBlock::BlockMap newEntries;
        for (const auto& e : entries)
        {
            newEntries.insert_or_assign(e.first, std::make_unique<TdfBlock>(*e.second));
        }
        return newEntries;
    }

    TdfBlock::TdfBlock(const TdfBlock& other) : properties(other.properties), blocks(copyEntries(other.blocks))
    {
    }

    TdfBlock& TdfBlock::operator=(const TdfBlock& other)
    {
        properties = other.properties;
        blocks = copyEntries(other.blocks);
        return *this;
    }

    void TdfBlock::insertOrAssignProperty(const std::string& name, const std::string& value)
    {
        properties.insert_or_assign(name, value);
    }

    TdfBlock& TdfBlock::insertOrAssignBlock(const std::string& name, const TdfBlock& block)
    {
        auto pair = blocks.insert_or_assign(name, std::make_unique<TdfBlock>(block));
        return *pair.first->second;
    }

    TdfBlock& TdfBlock::createBlock(const std::string& name)
    {
        auto pair = blocks.insert_or_assign(name, std::make_unique<TdfBlock>());
        return *pair.first->second;
    }

    struct MakeBlockVisitor : public boost::static_visitor<>
    {
        TdfBlock* block;
        const std::string* name;

        MakeBlockVisitor(TdfBlock* block, const std::string* name) : block(block), name(name)
        {
        }

        void operator()(const std::string& value) const
        {
            block->insertOrAssignProperty(*name, value);
        }

        void operator()(const TdfBlock& value) const
        {
            block->insertOrAssignBlock(*name, value);
        }
    };

    TdfBlock makeTdfBlock(std::vector<std::pair<std::string, TdfPropertyValue::ValueType>> list)
    {
        TdfBlock block;
        for (const auto& item : list)
        {
            boost::apply_visitor(MakeBlockVisitor(&block, &item.first), item.second);
        }
        return block;
    }

    bool TdfPropertyValue::operator==(const TdfPropertyValue& rhs) const
    {
        return boost::apply_visitor(EqualityVisitor(&rhs.value), value);
    }
}
