#pragma once

#include <array>
#include <memory>
#include <rwe/UiRenderService.h>
#include <rwe/render/GraphicsContext.h>
#include <rwe/render/SpriteSeries.h>
#include <rwe/rwe_time.h>
#include <rwe/sdl/SdlContext.h>

namespace rwe
{
    enum class CursorType : size_t
    {
        Normal,
        Select,
        Attack,
        Move,
        Guard,
        Repair,
        Red,
        Green,
        NUM_CURSORS
    };

    using Cursors = std::array<std::shared_ptr<SpriteSeries>, static_cast<size_t>(CursorType::NUM_CURSORS)>;

    size_t operator*(CursorType t);

    class CursorService
    {
    private:
        SdlContext* sdlContext;
        TimeService* timeService;

        const Cursors _cursors;

        SpriteSeries* currentCursor;

    public:
        CursorService(SdlContext* sdlContext, TimeService* timeService, Cursors cursors);

        void useCursor(CursorType type);

        std::shared_ptr<SpriteSeries> getCursor(CursorType type) const;

        void render(UiRenderService& renderer) const;
    };
}
