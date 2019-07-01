#ifndef RWE_CURSORSERVICE_H
#define RWE_CURSORSERVICE_H

#include <array>
#include <memory>
#include <rwe/GraphicsContext.h>
#include <rwe/SdlContextManager.h>
#include <rwe/SpriteSeries.h>
#include <rwe/TextureService.h>
#include <rwe/UiRenderService.h>
#include <rwe/rwe_time.h>

namespace rwe
{
    class CursorService
    {
    public:
        enum Type
        {
            move,
            grn,
            select,
            red,
            load,
            revive,
            defend,
            patrol,
            protect,
            repair,
            attack,
            normal,
            pickup,
            airstrike,
            teleport,
            reclamate,
            findsite,
            unload,
            hourglass,
            toofar,
            __count
        };

        CursorService(
            SdlContext&     sdlContext,
            TimeService&    timeService,
            TextureService& textureService);

        void useCursor(Type cursor);

        void render(UiRenderService& renderer) const;

    private:
        SdlContext&     _sdlContext;
        TimeService&    _timeService;
        TextureService& _textureService;

        std::array<std::shared_ptr<SpriteSeries>, Type::__count> _cursors;

        size_t _currentCursor;
    };
}

#endif
