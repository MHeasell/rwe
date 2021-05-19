#include "LoadingScene.h"
#include <boost/interprocess/streams/bufferstream.hpp>
#include <rwe/GameNetworkService.h>
#include <rwe/Index.h>
#include <rwe/MapTerrainGraphics.h>
#include <rwe/atlas_util.h>
#include <rwe/geometry/CollisionMesh.h>
#include <rwe/io/fbi/io.h>
#include <rwe/io/ota/ota.h>
#include <rwe/io/tdf/tdf.h>
#include <rwe/io/tnt/TntArchive.h>
#include <rwe/io/weapontdf/WeaponTdf.h>
#include <rwe/ui/UiLabel.h>

namespace rwe
{
    std::unordered_map<std::string, FeatureDefinition> loadAllFeatureDefinitions(AbstractVirtualFileSystem& vfs)
    {
        std::unordered_map<std::string, FeatureDefinition> features;

        auto files = vfs.getFileNamesRecursive("features", ".tdf");

        for (const auto& name : files)
        {
            auto bytes = vfs.readFile("features/" + name);
            if (!bytes)
            {
                throw std::runtime_error("Failed to read feature " + name);
            }

            std::string tdfString(bytes->data(), bytes->size());

            auto tdfRoot = parseTdfFromString(tdfString);
            for (const auto& e : tdfRoot.blocks)
            {
                auto featureDefinition = FeatureDefinition::fromTdf(*e.second);
                features.insert_or_assign(e.first, std::move(featureDefinition));
            }
        }

        return features;
    }

    GameParameters::GameParameters(const std::string& mapName, unsigned int schemaIndex)
        : mapName(mapName),
          schemaIndex(schemaIndex)
    {
    }

    LoadingScene::LoadingScene(
        const SceneContext& sceneContext,
        TdfBlock* audioLookup,
        AudioService::LoopToken&& bgm,
        GameParameters gameParameters)
        : sceneContext(sceneContext),
          scaledUiRenderService(sceneContext.graphics, sceneContext.shaders, UiCamera(640.0, 480.0f)),
          nativeUiRenderService(sceneContext.graphics, sceneContext.shaders, UiCamera(static_cast<float>(sceneContext.viewport->width()), static_cast<float>(sceneContext.viewport->height()))),
          audioLookup(audioLookup),
          bgm(std::move(bgm)),
          gameParameters(std::move(gameParameters)),
          uiFactory(sceneContext.textureService, sceneContext.audioService, audioLookup, sceneContext.vfs, sceneContext.pathMapping, sceneContext.viewport->width(), sceneContext.viewport->height())
    {
    }

    void LoadingScene::init()
    {
        auto backgroundSprite = sceneContext.textureService->getBitmapRegion(
            "Loadgame2bg",
            0.f,
            0.f,
            640.f,
            480.f);

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

        for (Index i = 0; i < getSize(categories); ++i)
        {
            int y = 136 + (static_cast<int>(i) * 42);
            auto label = std::make_unique<UiLabel>(90, y, 100, 12, categories[i], font);
            panel->appendChild(std::move(label));

            auto bar = std::make_unique<UiLightBar>(205, y, 351, 21, barSprite);
            bar->setPercentComplete(i / 5.0f); // for demo/debugging purposes
            bars.push_back(bar.get());
            panel->appendChild(std::move(bar));
        }

        // set up network
        for (Index i = 0; i < getSize(gameParameters.players); ++i)
        {
            const auto& p = gameParameters.players[i];
            if (!p)
            {
                continue;
            }
            const auto address = std::visit(GetNetworkAddressVisitor(), p->controller);
            if (!address)
            {
                continue;
            }

            networkService.addEndpoint(static_cast<int>(i), address->first, address->second);
        }
        networkService.start(gameParameters.localNetworkPort);

        sceneContext.sceneManager->setNextScene(createGameScene(gameParameters.mapName, gameParameters.schemaIndex));

        // wait for other players before starting
        networkService.setDoneLoading();
        networkService.waitForAllToBeReady();
    }

    void LoadingScene::render()
    {
        panel->render(scaledUiRenderService);
    }

    std::seed_seq seedFromGameParameters(const GameParameters& params)
    {
        std::vector<unsigned int> initialVec;
        std::copy(params.mapName.begin(), params.mapName.end(), std::back_inserter(initialVec));

        for (const auto& e : params.players)
        {
            if (!e)
            {
                initialVec.push_back(0);
                continue;
            }

            initialVec.push_back(e->color.value);
            initialVec.push_back(static_cast<int>(e->energy.value));
            initialVec.push_back(static_cast<int>(e->metal.value));
            if (e->name)
            {
                std::copy(e->name->begin(), e->name->end(), std::back_inserter(initialVec));
            }
            else
            {
                initialVec.push_back(0);
            }
        }

        return std::seed_seq(initialVec.begin(), initialVec.end());
    }

    std::unique_ptr<GameScene> LoadingScene::createGameScene(const std::string& mapName, unsigned int schemaIndex)
    {
        auto atlasInfo = createTextureAtlases(sceneContext.vfs, sceneContext.graphics, sceneContext.palette);
        MeshService meshService(sceneContext.vfs, sceneContext.graphics, std::move(atlasInfo.textureAtlasMap), std::move(atlasInfo.teamTextureAtlasMap), std::move(atlasInfo.colorAtlasMap));

        auto [unitDatabase, meshDatabase] = createUnitDatabase(meshService);

        auto otaRaw = sceneContext.vfs->readFile(std::string("maps/").append(mapName).append(".ota"));
        if (!otaRaw)
        {
            throw std::runtime_error("Failed to read OTA file");
        }

        std::string otaStr(otaRaw->begin(), otaRaw->end());
        auto ota = parseOta(parseTdfFromString(otaStr));

        // Load features
        auto featuresMap = loadAllFeatureDefinitions(*sceneContext.vfs);
        auto mapInfo = loadMap(featuresMap, mapName, ota, schemaIndex);
        GameSimulation simulation(std::move(mapInfo.terrain), mapInfo.surfaceMetal);
        for (auto& feature : mapInfo.features)
        {
            if (auto objectInfo = std::get_if<FeatureObjectInfo>(&feature.renderInfo); objectInfo != nullptr)
            {
                if (!unitDatabase.hasUnitModelDefinition(objectInfo->objectName))
                {
                    auto meshInfo = meshService.loadProjectileMesh(objectInfo->objectName);
                    unitDatabase.addUnitModelDefinition(objectInfo->objectName, std::move(meshInfo.modelDefinition));
                    for (const auto& m : meshInfo.pieceMeshes)
                    {
                        meshDatabase.addUnitPieceMesh(objectInfo->objectName, m.first, m.second);
                    }
                }
            }

            simulation.addFeature(std::move(feature));
        }
        auto seedSeq = seedFromGameParameters(gameParameters);
        simulation.rng.seed(seedSeq);

        auto minimap = sceneContext.textureService->getMinimap(mapName);

        auto worldViewportWidth = static_cast<float>(sceneContext.viewport->width() - GameScene::GuiSizeLeft - GameScene::GuiSizeRight);
        auto worldViewportHeight = static_cast<float>(sceneContext.viewport->height() - GameScene::GuiSizeTop - GameScene::GuiSizeBottom);

        CabinetCamera worldCamera(worldViewportWidth, worldViewportHeight);
        worldCamera.setPosition(Vector3f(0.0f, 0.0f, 0.0f));

        UiCamera worldUiCamera(worldViewportWidth, worldViewportHeight);

        UiCamera chromeUiCamera(static_cast<float>(sceneContext.viewport->width()), static_cast<float>(sceneContext.viewport->height()));

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

        for (int i = 0; i < getSize(gameParameters.players); ++i)
        {
            const auto& params = gameParameters.players[i];
            if (params)
            {
                auto playerType = std::visit(IsComputerVisitor(), params->controller) ? GamePlayerType::Computer : GamePlayerType::Human;
                GamePlayerInfo gpi{params->name, playerType, params->color, GamePlayerStatus::Alive, params->side, params->metal, params->energy};
                auto playerId = simulation.addPlayer(gpi);
                gamePlayers[i] = playerId;
                playerCommandService->registerPlayer(playerId);

                if (std::visit(IsHumanVisitor(), params->controller))
                {
                    if (localPlayerId)
                    {
                        throw std::runtime_error("Multiple local human players found");
                    }

                    localPlayerId = gamePlayers[i];
                }

                if (auto networkInfo = std::get_if<PlayerControllerTypeNetwork>(&params->controller); networkInfo != nullptr)
                {
                    endpointInfos.emplace_back(playerId, networkService.getEndpoint(i));
                }
            }
        }
        if (!localPlayerId)
        {
            throw std::runtime_error("No local player!");
        }

        auto gameNetworkService = std::make_unique<GameNetworkService>(*localPlayerId, std::stoi(gameParameters.localNetworkPort), endpointInfos, playerCommandService.get());

        UiRenderService worldUiRenderService(sceneContext.graphics, sceneContext.shaders, worldUiCamera);
        UiRenderService chromeUiRenderService(sceneContext.graphics, sceneContext.shaders, chromeUiCamera);

        auto minimapDots = sceneContext.textureService->getGafEntry("anims/FX.GAF", "radlogo");
        if (minimapDots->sprites.size() != 10)
        {
            throw std::runtime_error("Incorrect number of frames in anims/FX.GAF radlogo");
        }
        auto minimapDotHighlight = sceneContext.textureService->getGafEntry("anims/FX.GAF", "radlogohigh")->sprites.at(0);

        InGameSoundsInfo sounds;
        sounds.immediateOrders = lookUpSound("IMMEDIATEORDERS");
        sounds.specialOrders = lookUpSound("SPECIALORDERS");
        sounds.setFireOrders = lookUpSound("SETFIREORDERS");
        sounds.nextBuildMenu = lookUpSound("NEXTBUILDMENU");
        sounds.buildButton = lookUpSound("BUILDBUTTON");
        sounds.ordersButton = lookUpSound("ORDERSBUTTON");
        sounds.addBuild = lookUpSound("ADDBUILD");
        sounds.okToBuild = lookUpSound("OKTOBUILD");
        sounds.notOkToBuild = lookUpSound("NOTOKTOBUILD");
        sounds.selectMultipleUnits = lookUpSound("SelectMultipleUnits");

        auto consoleFont = sceneContext.textureService->getFont("fonts/CONSOLE.FNT");

        std::optional<std::ofstream> stateLogStream;
        if (gameParameters.stateLogFile)
        {
            stateLogStream = std::ofstream(*gameParameters.stateLogFile, std::ios::binary);
        }

        auto gameScene = std::make_unique<GameScene>(
            sceneContext,
            std::move(playerCommandService),
            std::move(meshDatabase),
            worldCamera,
            atlasInfo.textureAtlas,
            std::move(atlasInfo.teamTextureAtlases),
            std::move(worldUiRenderService),
            std::move(chromeUiRenderService),
            std::move(simulation),
            std::move(mapInfo.terrainGraphics),
            std::move(collisionService),
            std::move(unitDatabase),
            std::move(meshService),
            std::move(gameNetworkService),
            minimap,
            minimapDots,
            minimapDotHighlight,
            std::move(sounds),
            consoleFont,
            *localPlayerId,
            audioLookup,
            std::move(stateLogStream));

        const auto& schema = ota.schemas.at(schemaIndex);

        std::optional<SimVector> humanStartPos;

        for (Index i = 0; i < getSize(gameParameters.players); ++i)
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

            auto worldStartPos = gameScene->getTerrain().topLeftCoordinateToWorld(SimVector(simScalarFromInt(startPos.xPos), 0_ss, simScalarFromInt(startPos.zPos)));
            worldStartPos.y = gameScene->getTerrain().getHeightAt(worldStartPos.x, worldStartPos.z);

            if (*gamePlayers[i] == *localPlayerId)
            {
                humanStartPos = worldStartPos;
            }

            const auto& sideData = getSideData(player->side);
            std::optional<std::reference_wrapper<Unit>> commander = gameScene->spawnCompletedUnit(sideData.commander, *gamePlayers[i], worldStartPos);
            if (commander)
            {
                commander->get().energyStorage += Energy(player->energy);
                commander->get().metalStorage += Metal(player->metal);
            }
        }

        if (!humanStartPos)
        {
            throw std::runtime_error("No human player!");
        }

        gameScene->setCameraPosition(Vector3f(simScalarToFloat(humanStartPos->x), 0.0f, simScalarToFloat(humanStartPos->z)));

        return gameScene;
    }

    LoadingScene::LoadMapResult LoadingScene::loadMap(const std::unordered_map<std::string, FeatureDefinition>& featuresMap, const std::string& mapName, const OtaRecord& ota, unsigned int schemaIndex)
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
            std::move(heightGrid),
            simScalarFromUInt(tnt.getHeader().seaLevel));

        MapTerrainGraphics terrainGraphics(
            std::move(tileTextures),
            std::move(dataGrid));

        const auto& schema = ota.schemas.at(schemaIndex);

        auto featureTemplates = getFeatures(featuresMap, tnt);

        std::vector<MapFeature> features;

        for (int y = 0; y < mapAttributes.getHeight(); ++y)
        {
            for (int x = 0; x < mapAttributes.getWidth(); ++x)
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
                        auto pos = computeFeaturePosition(terrain, featureTemplate, x, y);
                        auto feature = createFeature(pos, featureTemplate);
                        features.push_back(std::move(feature));
                }
            }
        }

        // add features from the OTA schema
        for (const auto& f : schema.features)
        {
            const auto& featureTemplate = featuresMap.at(f.featureName);
            auto pos = computeFeaturePosition(terrain, featureTemplate, f.xPos, f.zPos);
            auto feature = createFeature(pos, featureTemplate);
            features.push_back(std::move(feature));
        }

        return LoadMapResult{std::move(terrain), static_cast<unsigned char>(schema.surfaceMetal), std::move(features), std::move(terrainGraphics)};
    }

    std::vector<TextureArrayRegion> LoadingScene::getTileTextures(TntArchive& tnt)
    {
        static const unsigned int tileWidth = 32;
        static const unsigned int tileHeight = 32;
        static const unsigned int mipMapLevels = 5;
        static const auto tilesPerTextureArray = 256;

        std::vector<TextureArrayRegion> tileTextures;

        std::vector<Color> textureArrayBuffer;
        textureArrayBuffer.reserve(tileWidth * tileHeight * tilesPerTextureArray);

        std::vector<SharedTextureArrayHandle> textureArrayHandles;

        // read the tile graphics into textures
        {
            tnt.readTiles([&](const char* tile) {
                if (textureArrayBuffer.size() == tileWidth * tileHeight * tilesPerTextureArray)
                {
                    SharedTextureArrayHandle handle(sceneContext.graphics->createTextureArray(tileWidth, tileHeight, mipMapLevels, textureArrayBuffer));
                    textureArrayHandles.push_back(std::move(handle));
                    textureArrayBuffer.clear();
                }

                for (unsigned int i = 0; i < (tileWidth * tileHeight); ++i)
                {
                    auto index = static_cast<unsigned char>(tile[i]);
                    textureArrayBuffer.push_back((*sceneContext.palette)[index]);
                }
            });
        }
        textureArrayHandles.emplace_back(sceneContext.graphics->createTextureArray(tileWidth, tileHeight, mipMapLevels, textureArrayBuffer));

        // populate the list of texture regions referencing the textures
        for (unsigned int i = 0; i < tnt.getHeader().numberOfTiles; ++i)
        {
            assert(textureArrayHandles.size() > i / tilesPerTextureArray);
            auto textureIndex = i / tilesPerTextureArray;
            auto tileIndex = i % tilesPerTextureArray;
            tileTextures.emplace_back(textureArrayHandles[textureIndex], tileIndex);
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

    std::vector<FeatureDefinition> LoadingScene::getFeatures(const std::unordered_map<std::string, FeatureDefinition>& featuresMap, TntArchive& tnt)
    {
        std::vector<FeatureDefinition> features;

        tnt.readFeatures([&](const auto& featureName) {
            const auto& feature = featuresMap.at(featureName);
            features.push_back(feature);
        });

        return features;
    }

    MapFeature LoadingScene::createFeature(const SimVector& pos, const FeatureDefinition& definition)
    {
        MapFeature f;
        f.footprintX = definition.footprintX;
        f.footprintZ = definition.footprintZ;
        f.height = simScalarFromUInt(definition.height);
        f.isBlocking = definition.blocking;
        f.isIndestructible = definition.indestructible;
        f.metal = definition.metal;
        f.position = pos;

        if (!definition.object.empty())
        {
            f.renderInfo = FeatureObjectInfo{definition.object};
        }
        else
        {
            FeatureSpriteInfo spriteInfo;
            spriteInfo.transparentAnimation = definition.animTrans;
            spriteInfo.transparentShadow = definition.shadTrans;
            if (!definition.fileName.empty() && !definition.seqName.empty())
            {
                spriteInfo.animation = sceneContext.textureService->getGafEntry("anims/" + definition.fileName + ".GAF", definition.seqName);
            }
            if (!spriteInfo.animation)
            {
                spriteInfo.animation = sceneContext.textureService->getDefaultSpriteSeries();
            }

            if (!definition.fileName.empty() && !definition.seqNameShad.empty())
            {
                // Some third-party features have broken shadow anim names (e.g. "empty"),
                // ignore them if they don't exist.
                spriteInfo.shadowAnimation = sceneContext.textureService->tryGetGafEntry("anims/" + definition.fileName + ".GAF", definition.seqNameShad);
            }
            f.renderInfo = std::move(spriteInfo);
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

    SimVector LoadingScene::computeFeaturePosition(
        const MapTerrain& terrain,
        const FeatureDefinition& featureDefinition,
        int x,
        int y) const
    {
        const auto& heightmap = terrain.getHeightMap();

        int height = 0;
        if (x < heightmap.getWidth() - 1 && y < heightmap.getHeight() - 1)
        {
            height = computeMidpointHeight(heightmap, x, y);
        }

        auto position = terrain.heightmapIndexToWorldCorner(x, y);
        position.y = simScalarFromInt(height);

        position.x += (simScalarFromInt(featureDefinition.footprintX) * MapTerrain::HeightTileWidthInWorldUnits) / 2_ss;
        position.z += (simScalarFromInt(featureDefinition.footprintZ) * MapTerrain::HeightTileHeightInWorldUnits) / 2_ss;

        return position;
    }

    int LoadingScene::computeMidpointHeight(const Grid<unsigned char>& heightmap, int x, int y)
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

    std::pair<UnitDatabase, MeshDatabase> LoadingScene::createUnitDatabase(MeshService& meshService)
    {
        UnitDatabase db;
        MeshDatabase meshDb;

        // read sound categories
        {
            auto path = sceneContext.pathMapping->gamedata + "/SOUND.TDF";
            auto bytes = sceneContext.vfs->readFile(path);
            if (!bytes)
            {
                throw std::runtime_error("Failed to read " + path);
            }

            std::string soundString(bytes->data(), bytes->size());
            auto sounds = parseSoundTdf(parseTdfFromString(soundString));
            for (auto& s : sounds)
            {
                const auto& c = s.second;
                preloadSound(db, c.select1);
                preloadSound(db, c.unitComplete);
                preloadSound(db, c.activate);
                preloadSound(db, c.deactivate);
                preloadSound(db, c.ok1);
                preloadSound(db, c.arrived1);
                preloadSound(db, c.cant1);
                preloadSound(db, c.underAttack);
                preloadSound(db, c.build);
                preloadSound(db, c.repair);
                preloadSound(db, c.working);
                preloadSound(db, c.cloak);
                preloadSound(db, c.uncloak);
                preloadSound(db, c.capture);
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
            auto path = sceneContext.pathMapping->gamedata + "/MOVEINFO.TDF";
            auto bytes = sceneContext.vfs->readFile(path);
            if (!bytes)
            {
                throw std::runtime_error("Failed to read " + path);
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
            auto weaponFiles = sceneContext.vfs->getFileNames(sceneContext.pathMapping->weapons, ".tdf");

            for (const auto& fileName : weaponFiles)
            {
                auto bytes = sceneContext.vfs->readFile(sceneContext.pathMapping->weapons + "/" + fileName);
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

                    if (!pair.second.model.empty())
                    {
                        auto meshInfo = meshService.loadProjectileMesh(pair.second.model);
                        db.addUnitModelDefinition(pair.second.model, std::move(meshInfo.modelDefinition));
                        for (const auto& m : meshInfo.pieceMeshes)
                        {
                            meshDb.addUnitPieceMesh(pair.second.model, m.first, m.second);
                        }
                    }

                    if (!pair.second.explosionGaf.empty() && !pair.second.explosionArt.empty())
                    {
                        auto anim = sceneContext.textureService->getGafEntry("anims/" + pair.second.explosionGaf + ".gaf", pair.second.explosionArt);
                        meshDb.addSpriteSeries(pair.second.explosionGaf, pair.second.explosionArt, anim);
                    }
                    if (!pair.second.waterExplosionGaf.empty() && !pair.second.waterExplosionArt.empty())
                    {
                        auto anim = sceneContext.textureService->getGafEntry("anims/" + pair.second.waterExplosionGaf + ".gaf", pair.second.waterExplosionArt);
                        meshDb.addSpriteSeries(pair.second.waterExplosionGaf, pair.second.waterExplosionArt, anim);
                    }

                    db.addWeapon(pair.first, std::move(pair.second));
                }
            }
        }

        // read unit FBIs
        {
            auto fbis = sceneContext.vfs->getFileNames(sceneContext.pathMapping->units, ".fbi");

            for (const auto& fbiName : fbis)
            {
                auto bytes = sceneContext.vfs->readFile(sceneContext.pathMapping->units + "/" + fbiName);
                if (!bytes)
                {
                    throw std::runtime_error("File in listing could not be read: " + fbiName);
                }

                std::string fbiString(bytes->data(), bytes->size());
                auto fbi = parseUnitFbi(parseTdfFromString(fbiString));

                // if it's a builder, also attempt to read its gui pages
                if (fbi.builder)
                {
                    auto guiPages = loadBuilderGui(fbi.unitName);
                    if (guiPages)
                    {
                        db.addBuilderGui(fbi.unitName, std::move(*guiPages));
                    }

                    // TODO: if no gui defined, attempt to build it dynamically?
                    // Need a database of download.tdf mappings first...
                }

                db.addUnitInfo(fbi.unitName, fbi);

                auto meshInfo = meshService.loadUnitMesh(fbi.objectName);
                db.addUnitModelDefinition(fbi.objectName, std::move(meshInfo.modelDefinition));
                for (const auto& m : meshInfo.pieceMeshes)
                {
                    meshDb.addUnitPieceMesh(fbi.objectName, m.first, m.second);
                }

                db.addSelectionMesh(fbi.objectName, std::make_shared<CollisionMesh>(std::move(meshInfo.selectionMesh.collisionMesh)));
                meshDb.addSelectionMesh(fbi.objectName, std::make_shared<GlMesh>(std::move(meshInfo.selectionMesh.visualMesh)));
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

        // preload smoke
        {
            auto anim = sceneContext.textureService->getGafEntry("anims/FX.GAF", "smoke 1");
            meshDb.addSpriteSeries("FX", "smoke 1", anim);
        }

        // preload weapon sprites
        {
            auto anim = sceneContext.textureService->getGafEntry("anims/FX.GAF", "cannonshell");
            meshDb.addSpriteSeries("FX", "cannonshell", anim);
        }
        {
            auto anim = sceneContext.textureService->getGafEntry("anims/FX.GAF", "plasmasm");
            meshDb.addSpriteSeries("FX", "plasmasm", anim);
        }
        {
            auto anim = sceneContext.textureService->getGafEntry("anims/FX.GAF", "plasmamd");
            meshDb.addSpriteSeries("FX", "plasmamd", anim);
        }
        {
            auto anim = sceneContext.textureService->getGafEntry("anims/FX.GAF", "ultrashell");
            meshDb.addSpriteSeries("FX", "ultrashell", anim);
        }
        {
            auto anim = sceneContext.textureService->getGafEntry("anims/FX.GAF", "flamestream");
            meshDb.addSpriteSeries("FX", "flamestream", anim);
        }

        return std::make_pair(db, meshDb);
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

    std::optional<AudioService::SoundHandle> LoadingScene::lookUpSound(const std::string& key)
    {
        auto soundBlock = audioLookup->findBlock(key);
        if (!soundBlock)
        {
            return std::nullopt;
        }

        auto soundName = soundBlock->get().findValue("sound");
        if (!soundName)
        {
            return std::nullopt;
        }

        return sceneContext.audioService->loadSound(*soundName);
    }

    std::optional<std::vector<std::vector<GuiEntry>>> LoadingScene::loadBuilderGui(const std::string& unitName)
    {
        std::vector<std::vector<GuiEntry>> entries;
        for (int i = 1; auto rawGui = sceneContext.vfs->readFile(sceneContext.pathMapping->guis + "/" + unitName + std::to_string(i) + ".GUI"); ++i)
        {
            auto parsedGui = parseGuiFromBytes(*rawGui);
            if (!parsedGui)
            {
                throw std::runtime_error("Failed to parse unit builder GUI: " + unitName + std::to_string(i));
            }
            entries.push_back(std::move(*parsedGui));
        }

        if (entries.empty())
        {
            return std::nullopt;
        }

        return entries;
    }
}
