#pragma once

#include <deque>
#include <mutex>
#include <rwe/GameHash.h>
#include <rwe/PlayerCommand.h>
#include <rwe/SceneTime.h>
#include <rwe/sim/GameTime.h>
#include <rwe/sim/PlayerId.h>
#include <unordered_map>
#include <vector>

namespace rwe
{
    class PlayerCommandService
    {
    private:
        mutable std::mutex mutex;
        std::unordered_map<PlayerId, std::deque<std::vector<PlayerCommand>>> commandBuffers;
        std::unordered_map<PlayerId, std::deque<GameHash>> gameTimeBuffers;

    public:
        std::optional<std::vector<std::pair<PlayerId, std::vector<PlayerCommand>>>> tryPopCommands();

        void pushCommands(PlayerId player, const std::vector<PlayerCommand>& commands);

        void pushHash(PlayerId player, const GameHash& gameHash);

        int bufferedCommandCount(PlayerId player) const;

        void registerPlayer(PlayerId playerId);

        bool checkHashes();
    };
}
