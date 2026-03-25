#pragma once

#include <SDL3/SDL.h>
#include <rwe/events.h>

namespace rwe
{
    class Scene
    {
    public:
        virtual void update(int millisecondsElapsed) {}

        virtual void init() {}

        virtual void render() {}

        virtual void onKeyDown(const SDL_KeyboardEvent& /*key*/) {}

        virtual void onKeyUp(const SDL_KeyboardEvent& /*key*/) {}

        virtual void onMouseDown(MouseButtonEvent /*event*/) {}

        virtual void onMouseUp(MouseButtonEvent /*event*/) {}

        virtual void onMouseMove(MouseMoveEvent /*event*/) {}

        virtual void onMouseWheel(MouseWheelEvent /*event*/) {}

        virtual ~Scene() = default;
    };
}
