#ifndef RWE_SIMPLETDFADAPTER_H
#define RWE_SIMPLETDFADAPTER_H

#include <boost/variant.hpp>
#include <rwe/tdf/TdfParser.h>
#include <stack>
#include <string>
#include <vector>
#include <memory>

namespace rwe
{
    struct TdfBlockEntry;
    using TdfBlock = std::vector<TdfBlockEntry>;
    using TdfPropertyValue = boost::variant<TdfBlock, std::string>;
    struct TdfBlockEntry
    {
        std::string name;
        std::unique_ptr<TdfPropertyValue> value;

        TdfBlockEntry(std::string name, const std::string& value) : name(std::move(name)), value(std::make_unique<TdfPropertyValue>(value)) {}
        TdfBlockEntry(std::string name, std::vector<TdfBlockEntry> entries) : name(std::move(name)), value(std::make_unique<TdfPropertyValue>(std::move(entries))) {}
        explicit TdfBlockEntry(std::string name) : name(std::move(name)), value(std::make_unique<TdfPropertyValue>(TdfBlock())) {}

        TdfBlockEntry(const TdfBlockEntry& other) : name(other.name), value(std::make_unique<TdfPropertyValue>(*other.value))
        {
        }

        bool operator==(const TdfBlockEntry& rhs) const
        {
            if (name != rhs.name)
            {
                return false;
            }

            if (*value != *rhs.value)
            {
                return false;
            }

            return true;
        }

        bool operator!=(const TdfBlockEntry& rhs) const
        {
            return !(rhs == *this);
        }

    };

    class SimpleTdfAdapter : public TdfAdapter<std::vector<TdfBlockEntry>>
    {
    private:
        std::vector<TdfBlockEntry> root;

        std::vector<std::vector<TdfBlockEntry>*> blockStack;

    public:
        void onStart() override;

        void onProperty(const std::string& name, const std::string& value) override;

        void onStartBlock(const std::string& name) override;

        void onEndBlock() override;

        Result onDone() override;
    };

    std::ostream& operator<<(std::ostream& os, const TdfBlockEntry& entry);
}

#endif
