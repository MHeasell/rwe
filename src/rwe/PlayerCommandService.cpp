#include "PlayerCommandService.h"

namespace rwe
{
    bool PlayerCommandService::hasCommands(PlayerId player) const
    {
        auto it = commandBuffers.find(player);
        if (it == commandBuffers.end())
        {
            return false;
        }

        return !it->second.empty();
    }

    const std::vector<PlayerCommand>& PlayerCommandService::getFrontCommands(PlayerId player) const
    {
        auto it = commandBuffers.find(player);
        if (it == commandBuffers.end())
        {
            throw std::runtime_error("Unknown player");
        }

        return it->second.front();
    }

    void PlayerCommandService::popCommands()
    {
        for (auto& p : commandBuffers)
        {
            assert(!p.second.empty());
            p.second.pop_front();
        }
    }

    void PlayerCommandService::pushCommands(PlayerId player, const std::vector<PlayerCommand>& commands)
    {
        commandBuffers[player].push_back(commands);
    }
}
