#ifndef RWE_CURSORSERVICE_H
#define RWE_CURSORSERVICE_H

#include <memory>
#include <rwe/SpriteSeries.h>
#include <rwe/GraphicsContext.h>
#include <rwe/SdlContextManager.h>

namespace rwe
{
    class CursorService
    {
    private:
        SdlContext* sdlContext;

        std::shared_ptr<SpriteSeries> _normalCursor;
        std::shared_ptr<SpriteSeries> _selectCursor;

        SpriteSeries* currentCursor;

    public:
        CursorService(
            SdlContext* sdlContext,
            std::shared_ptr<SpriteSeries> normalCursor,
            std::shared_ptr<SpriteSeries> selectCursor);

        void useNormalCursor();

        void useSelectCursor();

        void render(GraphicsContext& graphics) const;
    };
}

#endif
