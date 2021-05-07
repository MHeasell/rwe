#include "CursorService.h"

namespace rwe
{
    size_t operator*(CursorType t) {
        return static_cast<size_t>(t);
    }

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

    std::shared_ptr<SpriteSeries> CursorService::getCursor(CursorType type) const
    {
        return _cursors[*type];
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
