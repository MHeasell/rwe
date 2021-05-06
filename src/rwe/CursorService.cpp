#include "CursorService.h"

namespace rwe
{
    CursorService::CursorService(
        SdlContext* sdlContext,
        TimeService* timeService,
        Cursors cursors)
        : sdlContext(sdlContext),
          timeService(timeService),
          _cursors(std::move(cursors)),
          currentCursor(_cursors[*CursorType::Normal].get())
    {
    }

    void CursorService::useCursor(CursorType type)
    {
        currentCursor = _cursors[*type].get();
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
