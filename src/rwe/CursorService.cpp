#include "CursorService.h"

namespace rwe
{
    CursorService::CursorService(
        SdlContext* sdlContext,
        std::shared_ptr<SpriteSeries> normalCursor,
        std::shared_ptr<SpriteSeries> selectCursor)
        : sdlContext(sdlContext),
          _normalCursor(std::move(normalCursor)),
          _selectCursor(std::move(selectCursor)),
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

    void CursorService::render(UiRenderService& renderer) const
    {
        int x;
        int y;

        sdlContext->getMouseState(&x, &y);

        renderer.drawSprite(x, y, *(currentCursor->sprites[0]));
    }
}
