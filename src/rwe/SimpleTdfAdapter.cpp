#include "SimpleTdfAdapter.h"

namespace rwe
{

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
