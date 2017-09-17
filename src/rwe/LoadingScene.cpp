#include <rwe/ui/UiLabel.h>
#include "LoadingScene.h"

namespace rwe
{

    void LoadingScene::init()
    {
        auto backgroundSprite = textureService->getBitmapRegion(
            "Loadgame2bg",
            0,
            0,
            640,
            480);

        panel = std::make_unique<UiPanel>(0, 0, 640, 480, backgroundSprite);

        std::vector<std::string> categories{
            "Textures",
            "Terrain",
            "Units",
            "Animation",
            "3D Data",
            "Explosions"
        };

        auto font = textureService->getGafEntry("anims/hattfont12.gaf", "Haettenschweiler (120)");

        for (int i = 0; i < categories.size(); ++i)
        {
            int y = 136 + (i * 42);
            auto label = std::make_unique<UiLabel>(90, y, 100, 12, categories[i], font);
            panel->appendChild(std::move(label));
        }
    }

    void LoadingScene::render(GraphicsContext& context)
    {
        panel->render(context);
        cursor->render(context);
    }

    LoadingScene::LoadingScene(TextureService* textureService, CursorService* cursor)
        : textureService(textureService), cursor(cursor)
    {}
}
