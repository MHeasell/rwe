#ifndef RWE_CURSORSERVICE_H
#define RWE_CURSORSERVICE_H

#include <memory>
#include <rwe/GraphicsContext.h>
#include <rwe/SdlContextManager.h>
#include <rwe/SpriteSeries.h>
#include <rwe/UiRenderService.h>

namespace rwe
{
    class CursorService
    {
    private:
        SdlContext* sdlContext;

        std::shared_ptr<SpriteSeries> _normalCursor;
        std::shared_ptr<SpriteSeries> _selectCursor;
        std::shared_ptr<SpriteSeries> _attackCursor;

        SpriteSeries* currentCursor;

    public:
        CursorService(
            SdlContext* sdlContext,
            std::shared_ptr<SpriteSeries> normalCursor,
            std::shared_ptr<SpriteSeries> selectCursor,
            std::shared_ptr<SpriteSeries> attackCursor);

        void useNormalCursor();

        void useSelectCursor();

        void useAttackCursor();

        void render(UiRenderService& renderer) const;
    };
}

#endif
