#include "LoadingScene.h"
#include <boost/interprocess/streams/bufferstream.hpp>
#include <rwe/GameNetworkService.h>
#include <rwe/WeaponTdf.h>
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
        const SceneContext& sceneContext,
        MapFeatureService* featureService,
        AudioService::LoopToken&& bgm,
        GameParameters gameParameters)
        : sceneContext(sceneContext),
          featureService(featureService),
          scaledUiRenderService(sceneContext.graphics, sceneContext.shaders, UiCamera(640.0, 480.0f)),
          nativeUiRenderService(sceneContext.graphics, sceneContext.shaders, UiCamera(sceneContext.viewportService->width(), sceneContext.viewportService->height())),
          bgm(std::move(bgm)),
          gameParameters(std::move(gameParameters))
    {
    }

    void LoadingScene::init()
    {
        auto backgroundSprite = sceneContext.textureService->getBitmapRegion(
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

        auto font = sceneContext.textureService->getGafEntry("anims/hattfont12.gaf", "Haettenschweiler (120)");

        auto barSpriteSeries = sceneContext.textureService->getGuiTexture("", "LIGHTBAR");

        auto barSprite = barSpriteSeries
            ? (*barSpriteSeries)->sprites[0]
            : sceneContext.textureService->getDefaultSpriteSeries()->sprites[0];

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

        // set up network
        for (const auto& p : gameParameters.players)
        {
            if (!p)
            {
                continue;
            }
            const auto address = boost::apply_visitor(GetNetworkAddressVisitor(), p->controller);
            if (!address)
            {
                continue;
            }

            networkService.addEndpoint(address->first, address->second);
        }
        networkService.start(gameParameters.localNetworkInterface, gameParameters.localNetworkPort);

        featureService->loadAllFeatureDefinitions();
        sceneContext.sceneManager->setNextScene(createGameScene(gameParameters.mapName, gameParameters.schemaIndex));

        // wait for other players before starting
        networkService.setDoneLoading();
        networkService.waitForAllToBeReady();
    }

    void LoadingScene::render(GraphicsContext& context)
    {
        panel->render(scaledUiRenderService);
        sceneContext.cursor->render(nativeUiRenderService);
    }

    std::unique_ptr<GameScene> LoadingScene::createGameScene(const std::string& mapName, unsigned int schemaIndex)
    {
        auto otaRaw = sceneContext.vfs->readFile(std::string("maps/").append(mapName).append(".ota"));
        if (!otaRaw)
        {
            throw std::runtime_error("Failed to read OTA file");
        }

        std::string otaStr(otaRaw->begin(), otaRaw->end());
        auto ota = parseOta(parseTdfFromString(otaStr));

        auto simulation = createInitialSimulation(mapName, ota, schemaIndex);

        auto worldViewportWidth = sceneContext.viewportService->width() - GameScene::GuiSizeLeft - GameScene::GuiSizeRight;
        auto worldViewportHeight = sceneContext.viewportService->height() - GameScene::GuiSizeTop - GameScene::GuiSizeBottom;

        CabinetCamera worldCamera(worldViewportWidth, worldViewportHeight);
        worldCamera.setPosition(Vector3f(0.0f, 0.0f, 0.0f));

        UiCamera worldUiCamera(worldViewportWidth, worldViewportHeight);

        UiCamera chromeUiCamera(sceneContext.viewportService->width(), sceneContext.viewportService->height());

        auto meshService = MeshService::createMeshService(sceneContext.vfs, sceneContext.graphics, sceneContext.palette);

        auto unitDatabase = createUnitDatabase();

        MovementClassCollisionService collisionService;

        // compute cached walkable grids for each movement class
        {
            UnitDatabase::MovementClassIterator it = unitDatabase.movementClassBegin();
            UnitDatabase::MovementClassIterator end = unitDatabase.movementClassEnd();
            for (; it != end; ++it)
            {
                const auto& name = it->first;
                const auto& mc = it->second;
                collisionService.registerMovementClass(name, computeWalkableGrid(simulation, mc));
            }
        }

        std::optional<PlayerId> localPlayerId;

        auto playerCommandService = std::make_unique<PlayerCommandService>();

        std::array<std::optional<PlayerId>, 10> gamePlayers;
        std::vector<GameNetworkService::EndpointInfo> endpointInfos;

        boost::asio::io_service ioContext;
        boost::asio::ip::udp::resolver resolver(ioContext);

        for (int i = 0; i < 10; ++i)
        {
            const auto& params = gameParameters.players[i];
            if (params)
            {
                auto playerType = boost::apply_visitor(IsComputerVisitor(), params->controller) ? GamePlayerType::Computer : GamePlayerType::Human;
                GamePlayerInfo gpi{playerType, params->color, GamePlayerStatus::Alive};
                auto playerId = simulation.addPlayer(gpi);
                gamePlayers[i] = playerId;
                playerCommandService->registerPlayer(playerId);

                if (boost::apply_visitor(IsHumanVisitor(), params->controller))
                {
                    if (localPlayerId)
                    {
                        throw std::runtime_error("Multiple local human players found");
                    }

                    localPlayerId = gamePlayers[i];
                }

                if (auto networkInfo = boost::get<PlayerControllerTypeNetwork>(&params->controller); networkInfo != nullptr)
                {
                    endpointInfos.emplace_back(playerId, *resolver.resolve(boost::asio::ip::udp::resolver::query(networkInfo->host, networkInfo->port)));
                }
            }
        }
        if (!localPlayerId)
        {
            throw std::runtime_error("No local player!");
        }

        auto localEndpoint = *resolver.resolve(boost::asio::ip::udp::resolver::query(gameParameters.localNetworkInterface, gameParameters.localNetworkPort));
        auto gameNetworkService = std::make_unique<GameNetworkService>(localEndpoint, endpointInfos, playerCommandService.get());

        RenderService worldRenderService(sceneContext.graphics, sceneContext.shaders, worldCamera);
        UiRenderService worldUiRenderService(sceneContext.graphics, sceneContext.shaders, worldUiCamera);
        UiRenderService chromeUiRenderService(sceneContext.graphics, sceneContext.shaders, chromeUiCamera);

        auto gameScene = std::make_unique<GameScene>(
            sceneContext,
            std::move(playerCommandService),
            std::move(worldRenderService),
            std::move(worldUiRenderService),
            std::move(chromeUiRenderService),
            std::move(simulation),
            std::move(collisionService),
            std::move(unitDatabase),
            std::move(meshService),
            std::move(gameNetworkService),
            *localPlayerId);

        const auto& schema = ota.schemas.at(schemaIndex);

        std::optional<Vector3f> humanStartPos;

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
        auto tntBytes = sceneContext.vfs->readFile("maps/" + mapName + ".tnt");
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
            Vector3f pos = computeFeaturePosition(simulation.terrain, featureTemplate, f.xPos, f.zPos);
            auto feature = createFeature(pos, featureTemplate);
            simulation.addFeature(std::move(feature));
        }

        return simulation;
    }

    std::vector<TextureRegion> LoadingScene::getTileTextures(TntArchive& tnt)
    {
        static const unsigned int tileWidth = 32;
        static const unsigned int tileHeight = 32;
        static const unsigned int textureWidth = 1024;
        static const unsigned int textureHeight = 1024;
        static const auto textureWidthInTiles = textureWidth / tileWidth;
        static const auto textureHeightInTiles = textureHeight / tileHeight;
        static const auto tilesPerTexture = textureWidthInTiles * textureHeightInTiles;

        std::vector<TextureRegion> tileTextures;

        Grid<Color> textureBuffer(textureWidth, textureHeight);

        std::vector<SharedTextureHandle> textureHandles;

        // read the tile graphics into textures
        {
            unsigned int tileCount = 0;
            tnt.readTiles([this, &tileCount, &textureBuffer, &textureHandles](const char* tile) {
                if (tileCount == tilesPerTexture)
                {
                    SharedTextureHandle handle(sceneContext.graphics->createTexture(textureBuffer));
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
                        textureBuffer.set(textureX, textureY, (*sceneContext.palette)[index]);
                    }
                }

                tileCount += 1;
            });
        }
        textureHandles.emplace_back(sceneContext.graphics->createTexture(textureBuffer));

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
            f.animation = sceneContext.textureService->getGafEntry("anims/" + definition.fileName + ".GAF", definition.seqName);
        }
        if (!f.animation)
        {
            f.animation = sceneContext.textureService->getDefaultSpriteSeries();
        }

        if (!definition.fileName.empty() && !definition.seqNameShad.empty())
        {
            // Some third-party features have broken shadow anim names (e.g. "empty"),
            // ignore them if they don't exist.
            f.shadowAnimation = sceneContext.textureService->tryGetGafEntry("anims/" + definition.fileName + ".GAF", definition.seqNameShad);
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
        auto it = sceneContext.sideData->find(side);
        if (it == sceneContext.sideData->end())
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
            auto bytes = sceneContext.vfs->readFile("gamedata/SOUND.TDF");
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
            auto bytes = sceneContext.vfs->readFile("gamedata/MOVEINFO.TDF");
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

        // read weapons
        {
            auto weaponFiles = sceneContext.vfs->getFileNames("weapons", ".tdf");

            for (const auto& fileName : weaponFiles)
            {
                auto bytes = sceneContext.vfs->readFile("weapons/" + fileName);
                if (!bytes)
                {
                    throw std::runtime_error("File in listing could not be read: " + fileName);
                }

                std::string tdfString(bytes->data(), bytes->size());
                auto entries = parseWeaponTdf(parseTdfFromString(tdfString));

                for (auto& pair : entries)
                {
                    preloadSound(db, pair.second.soundStart);
                    preloadSound(db, pair.second.soundHit);
                    preloadSound(db, pair.second.soundWater);
                    db.addWeapon(pair.first, std::move(pair.second));
                }
            }
        }

        // read unit FBIs
        {
            auto fbis = sceneContext.vfs->getFileNames("units", ".fbi");

            for (const auto& fbiName : fbis)
            {
                auto bytes = sceneContext.vfs->readFile("units/" + fbiName);
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
            auto scripts = sceneContext.vfs->getFileNames("scripts", ".cob");

            for (const auto& scriptName : scripts)
            {
                auto bytes = sceneContext.vfs->readFile("scripts/" + scriptName);
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

    void LoadingScene::preloadSound(UnitDatabase& db, const std::optional<std::string>& soundName)
    {
        if (!soundName)
        {
            return;
        }

        preloadSound(db, *soundName);
    }

    void LoadingScene::preloadSound(UnitDatabase& db, const std::string& soundName)
    {
        auto sound = sceneContext.audioService->loadSound(soundName);
        if (!sound)
        {
            return; // sometimes sound categories name invalid sounds
        }

        db.addSound(soundName, *sound);
    }
}
