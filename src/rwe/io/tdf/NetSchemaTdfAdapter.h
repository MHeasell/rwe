#pragma once
#include <rwe/io/tdf/TdfParser.h>
#include <vector>

namespace rwe
{
    class NetSchemaTdfAdapter : public TdfAdapter<bool>
    {
    private:
        bool hasNetSchema;
        int schemaNestedLevel;
    public:
        void onStart() override;

        void onProperty(const std::string& name, const std::string& value) override;

        void onStartBlock(const std::string& name) override;

        void onEndBlock() override;

        Result onDone() override;
    };
}
