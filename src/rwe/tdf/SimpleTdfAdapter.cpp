#include "SimpleTdfAdapter.h"

namespace rwe
{
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
        auto& blockEntry = top->entries.emplace_back(name);
        blockStack.push_back(&boost::get<TdfBlock>(*(blockEntry.value)));
    }

    void SimpleTdfAdapter::onEndBlock()
    {
        blockStack.pop_back();
    }

    SimpleTdfAdapter::Result SimpleTdfAdapter::onDone()
    {
        return std::move(root);
    }
}
