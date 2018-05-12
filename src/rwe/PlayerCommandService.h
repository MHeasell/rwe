#ifndef RWE_GAMENETWORKSERVICE_H
#define RWE_GAMENETWORKSERVICE_H

#include <deque>
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
        std::unordered_map<PlayerId, std::deque<std::vector<PlayerCommand>>> commandBuffers;

    public:
        bool hasCommands(PlayerId player) const;

        const std::vector<PlayerCommand>& getFrontCommands(PlayerId player) const;

        void popCommands();

        void pushCommands(PlayerId player, const std::vector<PlayerCommand>& commands);
    };
}

#endif
