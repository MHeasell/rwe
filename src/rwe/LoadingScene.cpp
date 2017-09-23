#include <rwe/ui/UiLabel.h>
#include <rwe/tnt/TntArchive.h>
#include <boost/interprocess/streams/bufferstream.hpp>
#include "LoadingScene.h"

namespace rwe
{
    LoadingScene::LoadingScene(
        AbstractVirtualFileSystem* vfs,
        TextureService* textureService,
        CursorService* cursor,
        GraphicsContext* graphics,
        const ColorPalette* palette,
        SceneManager* sceneManager,
        AudioService::LoopToken&& bgm,
        GameParameters gameParameters)
        : vfs(vfs),
          textureService(textureService),
          cursor(cursor),
          graphics(graphics),
          palette(palette),
          sceneManager(sceneManager),
          bgm(std::move(bgm)),
          gameParameters(std::move(gameParameters))
    {
    }

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
            "Explosions"};

        auto font = textureService->getGafEntry("anims/hattfont12.gaf", "Haettenschweiler (120)");

        auto barSpriteSeries = textureService->getGuiTexture("", "LIGHTBAR");

        auto barSprite = barSpriteSeries
            ? (*barSpriteSeries)->sprites[0]
            : textureService->getDefaultSpriteSeries()->sprites[0];

        for (unsigned int i = 0; i < categories.size(); ++i)
        {
            int y = 136 + (i * 42);
            auto label = std::make_unique<UiLabel>(90, y, 100, 12, categories[i], font);
            panel->appendChild(std::move(label));

            auto bar = std::make_unique<UiLightBar>(205, y, 351, 21, barSprite);
            bar->setPercentComplete(i / 5.0f); // for demo/debugging purposes
            bars.push_back(bar.get());
            panel->appendChild(std::move(bar));
        }

        sceneManager->setNextScene(std::make_shared<GameScene>(createGameScene(gameParameters.mapName)));
    }

    void LoadingScene::render(GraphicsContext& context)
    {
        panel->render(context);
        cursor->render(context);
    }

    GameScene LoadingScene::createGameScene(const std::string& mapName)
    {
        return GameScene(cursor, rwe::CabinetCamera(640.0f, 480.0f), createMapTerrain(mapName));
    }

    MapTerrain LoadingScene::createMapTerrain(const std::string& mapName)
    {
        auto tntBytes = vfs->readFile("maps/" + mapName + ".tnt");
        if (!tntBytes)
        {
            throw std::runtime_error("Failed to load map bytes");
        }

        boost::interprocess::bufferstream tntStream(tntBytes->data(), tntBytes->size());
        TntArchive tnt(&tntStream);

        const unsigned int tileWidth = 32;
        const unsigned int tileHeight = 32;
        const unsigned int textureWidth = 1024;
        const unsigned int textureHeight = 1024;
        const auto textureWidthInTiles = textureWidth / tileWidth;
        const auto textureHeightInTiles = textureHeight / tileHeight;
        const auto tilesPerTexture = textureWidthInTiles * textureHeightInTiles;

        std::vector<TextureRegion> tileTextures;

        std::vector<Color> textureBuffer(textureWidth * textureHeight);

        std::vector<SharedTextureHandle> textureHandles;

        // read the tile graphics into textures
        {
            unsigned int tileCount = 0;
            tnt.readTiles([this, &tileCount, &textureBuffer, &textureHandles](const char* tile) {
                if (tileCount == tilesPerTexture)
                {
                    textureHandles.push_back(graphics->createTexture(textureWidth, textureHeight, textureBuffer));
                    tileCount = 0;
                }

                auto tileX = tileCount % textureWidthInTiles;
                auto tileY = tileCount / textureWidthInTiles;
                auto startX = tileX * tileWidth;
                auto startY = tileY * tileHeight;
                for (unsigned int dy = 0; dy < tileHeight; ++dy)
                {
                    for (unsigned int dx = 0; dx < tileWidth; ++dx)
                    {
                        auto textureX = startX + dx;
                        auto textureY = startY + dy;
                        auto index = static_cast<unsigned char>(tile[(dy * tileWidth) + dx]);
                        textureBuffer[(textureY * textureWidth) + textureX] = (*palette)[index];
                    }
                }

                tileCount += 1;
            });
        }
        textureHandles.push_back(graphics->createTexture(textureWidth, textureHeight, textureBuffer));

        // populate the list of texture regions referencing the textures
        for (unsigned int i = 0; i < tnt.getHeader().numberOfTiles; ++i)
        {
            const float regionWidth = static_cast<float>(tileWidth) / static_cast<float>(textureWidth);
            const float regionHeight = static_cast<float>(tileHeight) / static_cast<float>(textureHeight);
            auto x = i % textureWidthInTiles;
            auto y = i / textureWidthInTiles;

            assert(textureHandles.size() > i / tilesPerTexture);
            tileTextures.emplace_back(
                textureHandles[i / tilesPerTexture],
                Rectangle2f::fromTopLeft(x * regionWidth, y * regionHeight, regionWidth, regionHeight));
        }

        auto mapWidthInTiles = tnt.getHeader().width / 2;
        auto mapHeightInTiles = tnt.getHeader().height / 2;
        std::vector<uint16_t> mapData(mapWidthInTiles * mapHeightInTiles);
        tnt.readMapData(mapData.data());
        std::vector<std::size_t> dataCopy;
        dataCopy.reserve(mapData.size());
        std::copy(mapData.begin(), mapData.end(), std::back_inserter(dataCopy));
        Grid<std::size_t> dataGrid(mapWidthInTiles, mapHeightInTiles, std::move(dataCopy));

        return MapTerrain(
            std::move(tileTextures),
            std::move(dataGrid),
            rwe::Grid<unsigned char>());
    }
}
