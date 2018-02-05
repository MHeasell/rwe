#include "CursorService.h"

namespace rwe
{
    CursorService::CursorService(
        SdlContext* sdlContext,
        std::shared_ptr<SpriteSeries> normalCursor,
        std::shared_ptr<SpriteSeries> selectCursor,
        std::shared_ptr<SpriteSeries> attackCursor)
        : sdlContext(sdlContext),
          _normalCursor(std::move(normalCursor)),
          _selectCursor(std::move(selectCursor)),
          _attackCursor(std::move(attackCursor)),
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

    void CursorService::render(UiRenderService& renderer) const
    {
        int x;
        int y;

        sdlContext->getMouseState(&x, &y);

        renderer.drawSprite(x, y, *(currentCursor->sprites[0]));
    }
}
