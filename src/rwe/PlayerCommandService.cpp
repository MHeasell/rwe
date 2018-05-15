#include "PlayerCommandService.h"

namespace rwe
{
    std::optional<std::vector<std::pair<PlayerId, std::vector<PlayerCommand>>>> PlayerCommandService::tryPopCommands()
    {
        std::scoped_lock<std::mutex> lock(mutex);

        for (const auto& p : commandBuffers)
        {
            if (p.second.empty())
            {
                return std::nullopt;
            }
        }

        std::vector<std::pair<PlayerId, std::vector<PlayerCommand>>> out;
        for (auto& p : commandBuffers)
        {
            out.emplace_back(p.first, p.second.front());
            p.second.pop_front();
        }

        return out;
    }

    void PlayerCommandService::pushCommands(PlayerId player, const std::vector<PlayerCommand>& commands)
    {
        std::scoped_lock<std::mutex> lock(mutex);

        commandBuffers.at(player).push_back(commands);
    }

    void PlayerCommandService::registerPlayer(PlayerId playerId)
    {
        std::scoped_lock<std::mutex> lock(mutex);

        auto result = commandBuffers.emplace(playerId, std::deque<std::vector<PlayerCommand>>());
        if (!result.second)
        {
            throw std::logic_error("Player already registered");
        }
    }
}
