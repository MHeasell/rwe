#pragma once

#include <rwe/SceneTime.h>
#include <rwe/rwe_time.h>


namespace rwe
{
    float ema(float val, float average, float alpha);

    template <typename Range>
    unsigned int estimateAverageSceneTimeStatic(SceneTime localSceneTime, Range sceneTimes, Timestamp time)
    {
        auto accum = localSceneTime.value;
        auto count = 1;
        for (const auto& lastKnownSceneTime : sceneTimes)
        {
            auto elapsedTimeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(time - lastKnownSceneTime.second).count();
            auto extraFrames = elapsedTimeMillis / 16;

            auto peerSceneTime = lastKnownSceneTime.first.value + extraFrames;
            accum += peerSceneTime;
            count += 1;
        }
        return accum / count;
    }
}
