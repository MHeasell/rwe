#include "CursorService.h"

namespace rwe
{
    CursorService::CursorService(
        SdlContext&     sdlContext,
        TimeService&    timeService,
        TextureService& textureService)
        : _sdlContext(sdlContext),
          _timeService(timeService),
          _textureService(textureService),
          _currentCursor(Type::normal)
    {
        _cursors[Type::move]      = textureService.getGafEntry("anims/CURSORS.GAF", "cursormove");
        _cursors[Type::grn]       = textureService.getGafEntry("anims/CURSORS.GAF", "cursorgrn");
        _cursors[Type::select]    = textureService.getGafEntry("anims/CURSORS.GAF", "cursorselect");
        _cursors[Type::red]       = textureService.getGafEntry("anims/CURSORS.GAF", "cursorred");
        _cursors[Type::load]      = textureService.getGafEntry("anims/CURSORS.GAF", "cursorload");
        _cursors[Type::revive]    = textureService.getGafEntry("anims/CURSORS.GAF", "cursorrevive");
        _cursors[Type::defend]    = textureService.getGafEntry("anims/CURSORS.GAF", "cursordefend");
        _cursors[Type::patrol]    = textureService.getGafEntry("anims/CURSORS.GAF", "cursorpatrol");
        _cursors[Type::protect]   = textureService.getGafEntry("anims/CURSORS.GAF", "cursorprotect");
        _cursors[Type::repair]    = textureService.getGafEntry("anims/CURSORS.GAF", "cursorrepair");
        _cursors[Type::attack]    = textureService.getGafEntry("anims/CURSORS.GAF", "cursorattack");
        _cursors[Type::normal]    = textureService.getGafEntry("anims/CURSORS.GAF", "cursornormal");
        _cursors[Type::pickup]    = textureService.getGafEntry("anims/CURSORS.GAF", "cursorpickup");
        _cursors[Type::airstrike] = textureService.getGafEntry("anims/CURSORS.GAF", "cursorairstrike");
        _cursors[Type::teleport]  = textureService.getGafEntry("anims/CURSORS.GAF", "cursorteleport");
        _cursors[Type::reclamate] = textureService.getGafEntry("anims/CURSORS.GAF", "cursorreclamate");
        _cursors[Type::findsite]  = textureService.getGafEntry("anims/CURSORS.GAF", "cursorfindsite");
        _cursors[Type::unload]    = textureService.getGafEntry("anims/CURSORS.GAF", "cursorunload");
        _cursors[Type::hourglass] = textureService.getGafEntry("anims/CURSORS.GAF", "cursorhourglass");
        _cursors[Type::toofar]    = textureService.getGafEntry("anims/CURSORS.GAF", "cursortoofar");
    }

    void CursorService::useCursor(Type cursor)
    {
        _currentCursor = cursor;
    }

    void CursorService::render(UiRenderService& renderer) const
    {
        int x;
        int y;

        _sdlContext.getMouseState(&x, &y);

        auto         timeInMillis       = _timeService.getTicks();
        const auto&  frames             = _cursors[_currentCursor]->sprites;
        unsigned int frameRateInSeconds = 6;
        unsigned int millisPerFrame     = 1000 / frameRateInSeconds;

        auto frameIndex = (timeInMillis / millisPerFrame) % frames.size();

        renderer.drawSprite(x, y, *(frames[frameIndex]));
    }
}
