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
          currentCursor(_selectCursor.get())
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

    void CursorService::render(GraphicsContext& graphics) const
    {
        int x;
        int y;

        sdlContext->getMouseState(&x, &y);

        graphics.drawSprite(x, y, currentCursor->sprites[0]);
    }
}
