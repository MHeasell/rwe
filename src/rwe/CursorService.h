#pragma once

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
        std::shared_ptr<SpriteSeries> _repairCursor;
        std::shared_ptr<SpriteSeries> _redCursor;
        std::shared_ptr<SpriteSeries> _greenCursor;

        SpriteSeries* currentCursor;

    public:
        CursorService(
            SdlContext* sdlContext,
            TimeService* timeService,
            std::shared_ptr<SpriteSeries> normalCursor,
            std::shared_ptr<SpriteSeries> selectCursor,
            std::shared_ptr<SpriteSeries> attackCursor,
            std::shared_ptr<SpriteSeries> moveCursor,
            std::shared_ptr<SpriteSeries> repairCursor,
            std::shared_ptr<SpriteSeries> redCursor,
            std::shared_ptr<SpriteSeries> greenCursor);

        void useNormalCursor();

        void useSelectCursor();

        void useAttackCursor();

        void useMoveCursor();

        void useRepairCursor();

        void useRedCursor();

        void useGreenCursor();

        void render(UiRenderService& renderer) const;
    };
}
