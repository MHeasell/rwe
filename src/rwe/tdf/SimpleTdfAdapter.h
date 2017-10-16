#ifndef RWE_SIMPLETDFADAPTER_H
#define RWE_SIMPLETDFADAPTER_H

#include <boost/variant.hpp>
#include <rwe/tdf/TdfParser.h>
#include <stack>
#include <string>
#include <vector>
#include <memory>
#include <rwe/tdf/TdfBlock.h>

namespace rwe
{
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
