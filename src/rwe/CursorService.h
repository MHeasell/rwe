#ifndef RWE_CURSORSERVICE_H
#define RWE_CURSORSERVICE_H

#include <memory>
#include <rwe/GraphicsContext.h>
#include <rwe/SdlContextManager.h>
#include <rwe/SpriteSeries.h>
#include <rwe/UiRenderService.h>
#include <rwe/rwe_time.h>

namespace rwe
{
    class CursorService
    {
    private:
        SdlContext* sdlContext;
        TimeService* timeService;

        std::shared_ptr<SpriteSeries> _normalCursor;
        std::shared_ptr<SpriteSeries> _selectCursor;
        std::shared_ptr<SpriteSeries> _attackCursor;
        std::shared_ptr<SpriteSeries> _moveCursor;
        std::shared_ptr<SpriteSeries> _redCursor;

        SpriteSeries* currentCursor;

    public:
        CursorService(
            SdlContext* sdlContext,
            TimeService* timeService,
            std::shared_ptr<SpriteSeries> normalCursor,
            std::shared_ptr<SpriteSeries> selectCursor,
            std::shared_ptr<SpriteSeries> attackCursor,
            std::shared_ptr<SpriteSeries> moveCursor,
            std::shared_ptr<SpriteSeries> redCursor);

        void useNormalCursor();

        void useSelectCursor();

        void useAttackCursor();

        void useMoveCursor();

        void useRedCursor();

        void render(UiRenderService& renderer) const;
    };
}

#endif
