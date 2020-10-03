#pragma once

#include <rwe/io/tdf/TdfBlock.h>
#include <rwe/io/tdf/TdfParser.h>
#include <string>
#include <vector>

namespace rwe
{
    class ListTdfAdapter : public TdfAdapter<std::vector<TdfBlock>>
    {
    private:
        std::vector<TdfBlock> root;

        std::vector<TdfBlock*> blockStack;

    public:
        void onStart() override;

        void onProperty(const std::string& name, const std::string& value) override;

        void onStartBlock(const std::string& name) override;

        void onEndBlock() override;

        Result onDone() override;
    };
}
