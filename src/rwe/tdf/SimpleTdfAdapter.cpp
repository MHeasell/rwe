#include "SimpleTdfAdapter.h"

namespace rwe
{
    class EqualityVisitor : public boost::static_visitor<bool>
    {
    private:
        const TdfPropertyValue* other;
    public:
        explicit EqualityVisitor(const TdfPropertyValue* other): other(other) {}
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

    void SimpleTdfAdapter::onStart()
    {
        root.entries.clear();
        blockStack.clear();
        blockStack.push_back(&root);
    }

    void SimpleTdfAdapter::onProperty(const std::string& name, const std::string& value)
    {
        blockStack.back()->entries.emplace_back(name, value);
    }

    void SimpleTdfAdapter::onStartBlock(const std::string& name)
    {
        auto top = blockStack.back();
        top->entries.emplace_back(name);
        blockStack.push_back(&boost::get<TdfBlock>(*(top->entries.back().value)));
    }

    void SimpleTdfAdapter::onEndBlock()
    {
        blockStack.pop_back();
    }

    SimpleTdfAdapter::Result SimpleTdfAdapter::onDone()
    {
        return std::move(root);
    }

    TdfBlockEntry::TdfBlockEntry(std::string name, const std::string &value) : name(std::move(name)), value(std::make_unique<TdfPropertyValue>(value)) {}

    TdfBlockEntry::TdfBlockEntry(std::string name, TdfBlock block) : name(std::move(name)), value(std::make_unique<TdfPropertyValue>(std::move(block))) {}

    TdfBlockEntry::TdfBlockEntry(std::string name) : name(std::move(name)), value(std::make_unique<TdfPropertyValue>(TdfBlock())) {}

    TdfBlockEntry::TdfBlockEntry(const TdfBlockEntry &other) : name(other.name), value(std::make_unique<TdfPropertyValue>(*other.value))
    {
    }

    bool TdfBlockEntry::operator==(const TdfBlockEntry &rhs) const {
        if (name != rhs.name)
        {
            return false;
        }

        return boost::apply_visitor(EqualityVisitor(rhs.value.get()), *value);
    }

    bool TdfBlockEntry::operator!=(const TdfBlockEntry &rhs) const {
        return !(rhs == *this);
    }

    TdfBlockEntry::TdfBlockEntry(std::string name, std::vector<TdfBlockEntry> entries) : TdfBlockEntry(std::move(name), TdfBlock(std::move(entries)))
    {
    }

    std::ostream& operator<<(std::ostream& os, const TdfBlockEntry& entry)
    {
        auto leaf = boost::get<std::string>(&*entry.value);
        if (leaf != nullptr)
        {
            os << entry.name << " = " << *leaf << ";";
        }
        else
        {
            auto block = boost::get<TdfBlock>(&*entry.value);
            if (block != nullptr)
            {
                os << "[" << entry.name << "]{ ";
                for (auto elem : block->entries)
                {
                    os << elem << " ";
                }
                os << "}";
            }
            else
            {
                throw std::logic_error("Value was neither primitive nor block");
            }
        }

        return os;
    }

    bool TdfBlock::operator==(const TdfBlock& rhs) const
    {
        return entries == rhs.entries;
    }

    bool TdfBlock::operator!=(const TdfBlock& rhs) const
    {
        return !(rhs == *this);
    }

    boost::optional<const TdfBlock&> TdfBlock::findBlock(const std::string& name) const
    {
        // find the key in the block
        auto pos = std::find_if(entries.begin(), entries.end(), [name](const TdfBlockEntry& e) { return e.name == name; });
        if (pos == entries.end())
        {
            return boost::none;
        }

        // make sure the key contains a block and extract it
        auto& valuePointer = pos->value;
        return boost::get<TdfBlock>(*valuePointer);
    }

    boost::optional<const std::string&> TdfBlock::findValue(const std::string& name) const
    {
        // find the key in the block
        auto pos = std::find_if(entries.begin(), entries.end(), [name](const TdfBlockEntry& e) { return e.name == name; });
        if (pos == entries.end())
        {
            return boost::none;
        }

        // make sure the key contains a primitive (not a block) and extract it
        auto& valuePointer = pos->value;
        return boost::get<std::string>(*valuePointer);
    }

    TdfBlock::TdfBlock(std::vector<TdfBlockEntry>&& entries) : entries(std::move(entries))
    {
    }
}
