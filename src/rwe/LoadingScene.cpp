#include "LoadingScene.h"
#include <boost/interprocess/streams/bufferstream.hpp>
#include <rwe/ota.h>
#include <rwe/tdf.h>
#include <rwe/tnt/TntArchive.h>
#include <rwe/ui/UiLabel.h>

namespace rwe
{
    GameParameters::GameParameters(const std::string& mapName, unsigned int schemaIndex)
        : mapName(mapName),
          schemaIndex(schemaIndex)
    {
    }

    LoadingScene::LoadingScene(
        AbstractVirtualFileSystem* vfs,
        TextureService* textureService,
        AudioService* audioService,
        CursorService* cursor,
        GraphicsContext* graphics,
        ShaderService* shaders,
        MapFeatureService* featureService,
        const ColorPalette* palette,
        SceneManager* sceneManager,
        SdlContext* sdl,
        const std::unordered_map<std::string, SideData>* sideData,
        ViewportService* viewportService,
        AudioService::LoopToken&& bgm,
        GameParameters gameParameters)
        : vfs(vfs),
          textureService(textureService),
          audioService(audioService),
          cursor(cursor),
          graphics(graphics),
          shaders(shaders),
          featureService(featureService),
          palette(palette),
          sceneManager(sceneManager),
          sdl(sdl),
          sideData(sideData),
          viewportService(viewportService),
          scaledUiRenderService(graphics, shaders, UiCamera(640.0, 480.0f)),
          nativeUiRenderService(graphics, shaders, UiCamera(viewportService->width(), viewportService->height())),
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

        featureService->loadAllFeatureDefinitions();
        sceneManager->setNextScene(createGameScene(gameParameters.mapName, gameParameters.schemaIndex));
    }

    void LoadingScene::render(GraphicsContext& context)
    {
        panel->render(scaledUiRenderService);
        cursor->render(nativeUiRenderService);
    }

    std::unique_ptr<GameScene> LoadingScene::createGameScene(const std::string& mapName, unsigned int schemaIndex)
    {
        auto otaRaw = vfs->readFile(std::string("maps/").append(mapName).append(".ota"));
        if (!otaRaw)
        {
            throw std::runtime_error("Failed to read OTA file");
        }

        std::string otaStr(otaRaw->begin(), otaRaw->end());
        auto ota = parseOta(parseTdfFromString(otaStr));

        auto simulation = createInitialSimulation(mapName, ota, schemaIndex);

        CabinetCamera camera(viewportService->width(), viewportService->height());
        camera.setPosition(Vector3f(0.0f, 0.0f, 0.0f));

        UiCamera uiCamera(viewportService->width(), viewportService->height());

        auto meshService = MeshService::createMeshService(vfs, graphics, palette);

        auto unitDatabase = createUnitDatabase();

        UnitFactory unitFactory(std::move(unitDatabase), std::move(meshService));

        boost::optional<PlayerId> localPlayerId;

        std::array<boost::optional<PlayerId>, 10> gamePlayers;
        for (int i = 0; i < 10; ++i)
        {
            const auto& params = gameParameters.players[i];
            if (params)
            {
                GamePlayerInfo gpi{params->color};
                gamePlayers[i] = simulation.addPlayer(gpi);

                if (params->controller == PlayerInfo::Controller::Human)
                {
                    if (localPlayerId)
                    {
                        throw std::runtime_error("Multiple local human players found");
                    }

                    localPlayerId = gamePlayers[i];
                }
            }
        }
        if (!localPlayerId)
        {
            throw std::runtime_error("No local player!");
        }

        RenderService renderService(graphics, shaders, camera);
        UiRenderService uiRenderService(graphics, shaders, uiCamera);

        auto gameScene = std::make_unique<GameScene>(
            textureService,
            cursor,
            sdl,
            audioService,
            viewportService,
            std::move(renderService),
            std::move(uiRenderService),
            std::move(unitFactory),
            std::move(simulation),
            *localPlayerId);

        const auto& schema = ota.schemas.at(schemaIndex);

        boost::optional<Vector3f> humanStartPos;

        for (unsigned int i = 0; i < gameParameters.players.size(); ++i)
        {
            const auto& player = gameParameters.players[i];
            if (!player)
            {
                continue;
            }

            std::string startPosKey("StartPos");
            startPosKey.append(std::to_string(i + 1));

            auto startPosIt = std::find_if(schema.specials.begin(), schema.specials.end(), [&startPosKey](const OtaSpecial& s) { return s.specialWhat == startPosKey; });
            if (startPosIt == schema.specials.end())
            {
                throw std::runtime_error("Missing key from schema: " + startPosKey);
            }
            const auto& startPos = *startPosIt;

            auto worldStartPos = gameScene->getTerrain().topLeftCoordinateToWorld(Vector3f(startPos.xPos, 0.0f, startPos.zPos));
            worldStartPos.y = gameScene->getTerrain().getHeightAt(worldStartPos.x, worldStartPos.z);

            if (*gamePlayers[i] == *localPlayerId)
            {
                humanStartPos = worldStartPos;
            }

            const auto& sideData = getSideData(player->side);
            gameScene->spawnUnit(sideData.commander, *gamePlayers[i], worldStartPos);
        }

        if (!humanStartPos)
        {
            throw std::runtime_error("No human player!");
        }

        gameScene->setCameraPosition(Vector3f(humanStartPos->x, 0.0f, humanStartPos->z));

        return gameScene;
    }

    GameSimulation LoadingScene::createInitialSimulation(const std::string& mapName, const OtaRecord& ota, unsigned int schemaIndex)
    {
        auto tntBytes = vfs->readFile("maps/" + mapName + ".tnt");
        if (!tntBytes)
        {
            throw std::runtime_error("Failed to load map bytes");
        }

        boost::interprocess::bufferstream tntStream(tntBytes->data(), tntBytes->size());
        TntArchive tnt(&tntStream);

        auto tileTextures = getTileTextures(tnt);

        auto dataGrid = getMapData(tnt);

        Grid<TntTileAttributes> mapAttributes(tnt.getHeader().width, tnt.getHeader().height);
        tnt.readMapAttributes(mapAttributes.getData());

        auto heightGrid = getHeightGrid(mapAttributes);

        MapTerrain terrain(
            std::move(tileTextures),
            std::move(dataGrid),
            std::move(heightGrid),
            tnt.getHeader().seaLevel);

        GameSimulation simulation(std::move(terrain));

        auto featureTemplates = getFeatures(tnt);

        for (std::size_t y = 0; y < mapAttributes.getHeight(); ++y)
        {
            for (std::size_t x = 0; x < mapAttributes.getWidth(); ++x)
            {
                const auto& e = mapAttributes.get(x, y);
                switch (e.feature)
                {
                    case TntTileAttributes::FeatureNone:
                    case TntTileAttributes::FeatureUnknown:
                    case TntTileAttributes::FeatureVoid:
                        break;
                    default:
                        const auto& featureTemplate = featureTemplates[e.feature];
                        Vector3f pos = computeFeaturePosition(simulation.terrain, featureTemplate, x, y);
                        auto feature = createFeature(pos, featureTemplate);
                        simulation.addFeature(std::move(feature));
                }
            }
        }

        const auto& schema = ota.schemas.at(schemaIndex);

        // add features from the OTA schema
        for (const auto& f : schema.features)
        {
            const auto& featureTemplate = featureService->getFeatureDefinition(f.featureName);
            Vector3f pos = computeFeaturePosition(terrain, featureTemplate, f.xPos, f.zPos);
            auto feature = createFeature(pos, featureTemplate);
            simulation.addFeature(std::move(feature));
        }

        return simulation;
    }

    std::vector<TextureRegion> LoadingScene::getTileTextures(TntArchive& tnt)
    {
        const unsigned int tileWidth = 32;
        const unsigned int tileHeight = 32;
        const unsigned int textureWidth = 1024;
        const unsigned int textureHeight = 1024;
        const auto textureWidthInTiles = textureWidth / tileWidth;
        const auto textureHeightInTiles = textureHeight / tileHeight;
        const auto tilesPerTexture = textureWidthInTiles * textureHeightInTiles;

        std::vector<TextureRegion> tileTextures;

        Grid<Color> textureBuffer(textureWidth, textureHeight);

        std::vector<SharedTextureHandle> textureHandles;

        // read the tile graphics into textures
        {
            unsigned int tileCount = 0;
            tnt.readTiles([this, &tileCount, &textureBuffer, &textureHandles](const char* tile) {
                if (tileCount == tilesPerTexture)
                {
                    SharedTextureHandle handle(graphics->createTexture(textureBuffer));
                    textureHandles.push_back(std::move(handle));
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
                        textureBuffer.set(textureX, textureY, (*palette)[index]);
                    }
                }

                tileCount += 1;
            });
        }
        textureHandles.emplace_back(graphics->createTexture(textureBuffer));

        // populate the list of texture regions referencing the textures
        for (unsigned int i = 0; i < tnt.getHeader().numberOfTiles; ++i)
        {
            auto textureIndex = i / tilesPerTexture;
            auto tileIndex = i % tilesPerTexture;
            const float regionWidth = static_cast<float>(tileWidth) / static_cast<float>(textureWidth);
            const float regionHeight = static_cast<float>(tileHeight) / static_cast<float>(textureHeight);
            auto x = tileIndex % textureWidthInTiles;
            auto y = tileIndex / textureWidthInTiles;

            assert(textureHandles.size() > i / tilesPerTexture);
            tileTextures.emplace_back(
                textureHandles[textureIndex],
                Rectangle2f::fromTopLeft(x * regionWidth, y * regionHeight, regionWidth, regionHeight));
        }

        return tileTextures;
    }

    Grid<std::size_t> LoadingScene::getMapData(TntArchive& tnt)
    {
        auto mapWidthInTiles = tnt.getHeader().width / 2;
        auto mapHeightInTiles = tnt.getHeader().height / 2;
        std::vector<uint16_t> mapData(mapWidthInTiles * mapHeightInTiles);
        tnt.readMapData(mapData.data());
        std::vector<std::size_t> dataCopy;
        dataCopy.reserve(mapData.size());
        std::copy(mapData.begin(), mapData.end(), std::back_inserter(dataCopy));
        Grid<std::size_t> dataGrid(mapWidthInTiles, mapHeightInTiles, std::move(dataCopy));
        return dataGrid;
    }

    std::vector<FeatureDefinition> LoadingScene::getFeatures(TntArchive& tnt)
    {
        std::vector<FeatureDefinition> features;

        tnt.readFeatures([this, &features](const auto& featureName) {
            const auto& feature = featureService->getFeatureDefinition(featureName);
            features.push_back(feature);
        });

        return features;
    }

    MapFeature LoadingScene::createFeature(const Vector3f& pos, const FeatureDefinition& definition)
    {
        MapFeature f;
        f.footprintX = definition.footprintX;
        f.footprintZ = definition.footprintZ;
        f.height = definition.height;
        f.isBlocking = definition.blocking;
        f.position = pos;
        f.transparentAnimation = definition.animTrans;
        f.transparentShadow = definition.shadTrans;
        if (!definition.fileName.empty() && !definition.seqName.empty())
        {
            f.animation = textureService->getGafEntry("anims/" + definition.fileName + ".GAF", definition.seqName);
        }
        if (!f.animation)
        {
            f.animation = textureService->getDefaultSpriteSeries();
        }

        if (!definition.fileName.empty() && !definition.seqNameShad.empty())
        {
            // Some third-party features have broken shadow anim names (e.g. "empty"),
            // ignore them if they don't exist.
            f.shadowAnimation = textureService->tryGetGafEntry("anims/" + definition.fileName + ".GAF", definition.seqNameShad);
        }

        return f;
    }

    Grid<unsigned char> LoadingScene::getHeightGrid(const Grid<TntTileAttributes>& attrs) const
    {
        const auto& sourceData = attrs.getVector();

        std::vector<unsigned char> data;
        data.reserve(sourceData.size());

        std::transform(sourceData.begin(), sourceData.end(), std::back_inserter(data), [](const TntTileAttributes& e) {
            return e.height;
        });

        return Grid<unsigned char>(attrs.getWidth(), attrs.getHeight(), std::move(data));
    }

    Vector3f LoadingScene::computeFeaturePosition(
        const MapTerrain& terrain,
        const FeatureDefinition& featureDefinition,
        std::size_t x,
        std::size_t y) const
    {
        const auto& heightmap = terrain.getHeightMap();

        unsigned int height = 0;
        if (x < heightmap.getWidth() - 1 && y < heightmap.getHeight() - 1)
        {
            height = computeMidpointHeight(heightmap, x, y);
        }

        auto position = terrain.heightmapIndexToWorldCorner(x, y);
        position.y = height;

        position.x += (featureDefinition.footprintX * MapTerrain::HeightTileWidthInWorldUnits) / 2.0f;
        position.z += (featureDefinition.footprintZ * MapTerrain::HeightTileHeightInWorldUnits) / 2.0f;

        return position;
    }

    unsigned int LoadingScene::computeMidpointHeight(const Grid<unsigned char>& heightmap, std::size_t x, std::size_t y)
    {
        assert(x < heightmap.getWidth() - 1);
        assert(y < heightmap.getHeight() - 1);
        return (heightmap.get(x, y) + heightmap.get(x + 1, y) + heightmap.get(x, y + 1) + heightmap.get(x + 1, y + 1)) / 4u;
    }

    const SideData& LoadingScene::getSideData(const std::string& side) const
    {
        auto it = sideData->find(side);
        if (it == sideData->end())
        {
            throw std::runtime_error("Missing side data for " + side);
        }

        return it->second;
    }

    UnitDatabase LoadingScene::createUnitDatabase()
    {
        UnitDatabase db;

        // read sound categories
        {
            auto bytes = vfs->readFile("gamedata/SOUND.TDF");
            if (!bytes)
            {
                throw std::runtime_error("Failed to read gamedata/SOUND.TDF");
            }

            std::string soundString(bytes->data(), bytes->size());
            auto sounds = parseSoundTdf(parseTdfFromString(soundString));
            for (auto& s : sounds)
            {
                const auto& c = s.second;
                preloadSound(db, c.select1);
                preloadSound(db, c.ok1);
                preloadSound(db, c.arrived1);
                preloadSound(db, c.cant1);
                preloadSound(db, c.underAttack);
                preloadSound(db, c.count5);
                preloadSound(db, c.count4);
                preloadSound(db, c.count3);
                preloadSound(db, c.count2);
                preloadSound(db, c.count1);
                preloadSound(db, c.count0);
                preloadSound(db, c.cancelDestruct);
                db.addSoundClass(s.first, std::move(s.second));
            }
        }

        // read movement classes
        {
            auto bytes = vfs->readFile("gamedata/MOVEINFO.TDF");
            if (!bytes)
            {
                throw std::runtime_error("Failed to read gamedata/MOVEINFO.TDF");
            }

            std::string movementString(bytes->data(), bytes->size());
            auto classes = parseMovementTdf(parseTdfFromString(movementString));
            for (auto& c : classes)
            {
                auto name = c.second.name;
                db.addMovementClass(name, std::move(c.second));
            }
        }

        // read unit FBIs
        {
            auto fbis = vfs->getFileNames("units", ".fbi");

            for (const auto& fbiName : fbis)
            {
                auto bytes = vfs->readFile("units/" + fbiName);
                if (!bytes)
                {
                    throw std::runtime_error("File in listing could not be read: " + fbiName);
                }

                std::string fbiString(bytes->data(), bytes->size());
                auto fbi = parseUnitFbi(parseTdfFromString(fbiString));

                db.addUnitInfo(fbi.unitName, fbi);
            }
        }

        // read unit scripts
        {
            auto scripts = vfs->getFileNames("scripts", ".cob");

            for (const auto& scriptName : scripts)
            {
                auto bytes = vfs->readFile("scripts/" + scriptName);
                if (!bytes)
                {
                    throw std::runtime_error("File in listing could not be read: " + scriptName);
                }

                boost::interprocess::bufferstream s(bytes->data(), bytes->size());
                auto cob = parseCob(s);

                auto scriptNameWithoutExtension = scriptName.substr(0, scriptName.size() - 4);

                db.addUnitScript(scriptNameWithoutExtension, std::move(cob));
            }
        }

        return db;
    }

    void LoadingScene::preloadSound(UnitDatabase& db, const boost::optional<std::string>& soundName)
    {
        if (!soundName)
        {
            return;
        }

        preloadSound(db, *soundName);
    }

    void LoadingScene::preloadSound(UnitDatabase& db, const std::string& soundName)
    {
        auto sound = audioService->loadSound(soundName);
        if (!sound)
        {
            return; // sometimes sound categories name invalid sounds
        }

        db.addSound(soundName, *sound);
    }
}
