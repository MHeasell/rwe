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

    void PlayerCommandService::pushHash(PlayerId player, const GameHash& gameHash)
    {
        std::scoped_lock<std::mutex> lock(mutex);
        gameTimeBuffers.at(player).push_back(gameHash);
    }

    void PlayerCommandService::registerPlayer(PlayerId playerId)
    {
        std::scoped_lock<std::mutex> lock(mutex);

        auto result = commandBuffers.emplace(playerId, std::deque<std::vector<PlayerCommand>>());
        if (!result.second)
        {
            throw std::logic_error("Player already registered");
        }

        gameTimeBuffers.emplace(playerId, std::deque<GameHash>());
    }

    unsigned int PlayerCommandService::bufferedCommandCount(PlayerId player) const
    {
        std::scoped_lock<std::mutex> lock(mutex);

        return static_cast<unsigned int>(commandBuffers.at(player).size());
    }

    bool PlayerCommandService::checkHashes()
    {
        std::scoped_lock<std::mutex> lock(mutex);

        while (!std::any_of(gameTimeBuffers.begin(), gameTimeBuffers.end(), [](const auto& p) { return p.second.empty(); }))
        {
            std::optional<GameHash> baseHash;
            bool matching = true;
            for (auto& p : gameTimeBuffers)
            {
                auto hash = p.second.front();
                p.second.pop_front();

                if (!baseHash)
                {
                    baseHash = hash;
                    continue;
                }

                matching = matching && (*baseHash == hash);
            }

            if (!matching)
            {
                return false;
            }
        }

        return true;
    }
}
