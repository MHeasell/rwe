#ifndef RWE_SIMPLETDFADAPTER_H
#define RWE_SIMPLETDFADAPTER_H

#include <boost/variant.hpp>
#include <rwe/TdfParser.h>
#include <stack>
#include <string>
#include <vector>
#include <memory>

namespace rwe
{

    class SimpleTdfAdapter : public TdfAdapter
    {
    private:
    public:
        struct BlockEntry;
        using PropertyValue = boost::variant<std::vector<BlockEntry>, std::string>;
        struct BlockEntry
        {
            std::string name;
            std::unique_ptr<PropertyValue> value;

            BlockEntry(std::string name, const std::string& value) : name(std::move(name)), value(std::make_unique<PropertyValue>(value)) {}
            BlockEntry(std::string name, std::vector<BlockEntry> entries) : name(std::move(name)), value(std::make_unique<PropertyValue>(std::move(entries))) {}
            explicit BlockEntry(std::string name) : name(std::move(name)), value(std::make_unique<PropertyValue>(std::vector<BlockEntry>())) {}

            BlockEntry(const BlockEntry& other) : name(other.name), value(std::make_unique<PropertyValue>(*other.value))
            {
            }

            bool operator==(const BlockEntry& rhs) const
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
        };

    private:
        std::vector<BlockEntry> root;

        std::stack<std::vector<BlockEntry>*> blockStack;

    public:
        void onStart() override;

        void onProperty(const std::string& name, const std::string& value) override;

        void onStartBlock(const std::string& name) override;

        void onEndBlock() override;

        void onDone() override;

        const std::vector<BlockEntry>& getRoot() const;
    };

    std::ostream& operator<<(std::ostream& os, const SimpleTdfAdapter::BlockEntry& entry);
}

#endif
