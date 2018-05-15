#ifndef RWE_PLAYERCOMMANDSERVICE_H
#define RWE_PLAYERCOMMANDSERVICE_H

#include <deque>
#include <mutex>
#include <rwe/PlayerCommand.h>
#include <rwe/PlayerId.h>
#include <rwe/SceneTime.h>
#include <unordered_map>
#include <vector>

namespace rwe
{
    class PlayerCommandService
    {
    private:
        std::mutex mutex;
        std::unordered_map<PlayerId, std::deque<std::vector<PlayerCommand>>> commandBuffers;

    public:
        std::optional<std::vector<std::pair<PlayerId, std::vector<PlayerCommand>>>> tryPopCommands();

        void pushCommands(PlayerId player, const std::vector<PlayerCommand>& commands);

        void registerPlayer(PlayerId playerId);
    };
}

#endif
