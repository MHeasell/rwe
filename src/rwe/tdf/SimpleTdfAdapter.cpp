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
        root.clear();
        blockStack.clear();
        blockStack.push_back(&root);
    }

    void SimpleTdfAdapter::onProperty(const std::string& name, const std::string& value)
    {
        blockStack.back()->emplace_back(name, value);
    }

    void SimpleTdfAdapter::onStartBlock(const std::string& name)
    {
        auto top = blockStack.back();
        top->emplace_back(name);
        blockStack.push_back(&boost::get<std::vector<TdfBlockEntry>>(*top->back().value));
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

    TdfBlockEntry::TdfBlockEntry(std::string name, std::vector<TdfBlockEntry> entries) : name(std::move(name)), value(std::make_unique<TdfPropertyValue>(std::move(entries))) {}

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

    std::ostream& operator<<(std::ostream& os, const TdfBlockEntry& entry)
    {
        auto leaf = boost::get<std::string>(&*entry.value);
        if (leaf != nullptr)
        {
            os << entry.name << " = " << *leaf << ";";
        }
        else
        {
            auto block = boost::get<std::vector<TdfBlockEntry>>(&*entry.value);
            if (block != nullptr)
            {
                os << "[" << entry.name << "]{ ";
                for (auto elem : *block)
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
}
