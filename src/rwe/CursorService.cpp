#include "CursorService.h"

namespace rwe
{
    CursorService::CursorService(
        SdlContext* sdlContext,
        TimeService* timeService,
        std::shared_ptr<SpriteSeries> normalCursor,
        std::shared_ptr<SpriteSeries> selectCursor,
        std::shared_ptr<SpriteSeries> attackCursor,
        std::shared_ptr<SpriteSeries> redCursor)
        : sdlContext(sdlContext),
          timeService(timeService),
          _normalCursor(std::move(normalCursor)),
          _selectCursor(std::move(selectCursor)),
          _attackCursor(std::move(attackCursor)),
          _redCursor(std::move(redCursor)),
          currentCursor(_normalCursor.get())
    {
    }

    void CursorService::useNormalCursor()
    {
        currentCursor = _normalCursor.get();
    }

    void CursorService::useSelectCursor()
    {
        currentCursor = _selectCursor.get();
    }

    void CursorService::useAttackCursor()
    {
        currentCursor = _attackCursor.get();
    }

    void CursorService::useRedCursor()
    {
        currentCursor = _redCursor.get();
    }

    void CursorService::render(UiRenderService& renderer) const
    {
        int x;
        int y;

        sdlContext->getMouseState(&x, &y);

        auto timeInMillis = timeService->getTicks();
        const auto& frames = currentCursor->sprites;
        unsigned int frameRateInSeconds = 6;
        unsigned int millisPerFrame = 1000 / frameRateInSeconds;

        auto frameIndex = (timeInMillis / millisPerFrame) % frames.size();

        renderer.drawSprite(x, y, *(frames[frameIndex]));
    }
}
