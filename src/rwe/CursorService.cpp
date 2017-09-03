#include "CursorService.h"

namespace rwe
{
    void CursorService::render(GraphicsContext& graphics) const
    {
        int x;
        int y;

        sdlContext->getMouseState(&x, &y);

        graphics.drawSprite(x, y, cursor->sprites[0]);
    }

    CursorService::CursorService(SdlContext* sdlContext, std::shared_ptr<SpriteSeries> cursor)
            : sdlContext(sdlContext), cursor(std::move(cursor))
    {}
}
