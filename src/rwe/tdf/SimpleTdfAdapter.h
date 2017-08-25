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
    struct TdfBlock
    {
        std::vector<TdfBlockEntry> entries;

        TdfBlock() = default;

        explicit TdfBlock(std::vector<TdfBlockEntry>&& entries);

        boost::optional<const TdfBlock&> findBlock(const std::string& name) const;
        boost::optional<const std::string&> findValue(const std::string& name) const;

        bool operator==(const TdfBlock& rhs) const;

        bool operator!=(const TdfBlock& rhs) const;
    };
    using TdfPropertyValue = boost::variant<TdfBlock, std::string>;
    struct TdfBlockEntry
    {
        std::string name;
        std::unique_ptr<TdfPropertyValue> value;

        TdfBlockEntry(std::string name, const std::string& value);
        TdfBlockEntry(std::string name, TdfBlock block);
        TdfBlockEntry(std::string name, std::vector<TdfBlockEntry> entries);
        explicit TdfBlockEntry(std::string name);

        TdfBlockEntry(const TdfBlockEntry& other);

        bool operator==(const TdfBlockEntry& rhs) const;

        bool operator!=(const TdfBlockEntry& rhs) const;

    };

    class SimpleTdfAdapter : public TdfAdapter<TdfBlock>
    {
    private:
        TdfBlock root;

        std::vector<TdfBlock*> blockStack;

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
