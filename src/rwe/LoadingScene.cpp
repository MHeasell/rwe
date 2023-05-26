#include "LoadingScene.h"
#include <boost/interprocess/streams/bufferstream.hpp>
#include <rwe/LoadingScene_util.h>
#include <rwe/atlas_util.h>
#include <rwe/collections/SimpleVectorMap.h>
#include <rwe/game/FeatureMediaInfo.h>
#include <rwe/game/GameNetworkService.h>
#include <rwe/game/MapTerrainGraphics.h>
#include <rwe/geometry/CollisionMesh.h>
#include <rwe/io/fbi/io.h>
#include <rwe/io/featuretdf/io.h>
#include <rwe/io/moveinfotdf/MovementClassTdf.h>
#include <rwe/io/moveinfotdf/io.h>
#include <rwe/io/ota/ota.h>
#include <rwe/io/tdf/tdf.h>
#include <rwe/io/tnt/TntArchive.h>
#include <rwe/io/weapontdf/WeaponTdf.h>
#include <rwe/sim/FeatureDefinitionId.h>
#include <rwe/ui/UiLabel.h>
#include <rwe/util/Index.h>

namespace rwe
{
    const Viewport MenuUiViewport(0, 0, 640, 480);

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
            initialVec.push_back(e->energy.value);
            initialVec.push_back(e->metal.value);
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
          scaledUiRenderService(sceneContext.graphics, sceneContext.shaders, &MenuUiViewport),
          nativeUiRenderService(sceneContext.graphics, sceneContext.shaders, sceneContext.viewport),
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

        for (Index i = 0; i < getSize(categories); ++i)
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

            networkService.addEndpoint(i, address->first, address->second);
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


    std::unique_ptr<GameScene> LoadingScene::createGameScene(const std::string& mapName, unsigned int schemaIndex)
    {
        auto atlasInfo = createTextureAtlases(sceneContext.vfs, sceneContext.graphics, sceneContext.palette);
        MeshService meshService(sceneContext.vfs, sceneContext.graphics, std::move(atlasInfo.textureAtlasMap), std::move(atlasInfo.teamTextureAtlasMap), std::move(atlasInfo.colorAtlasMap));

        auto otaRaw = sceneContext.vfs->readFile(std::string("maps/").append(mapName).append(".ota"));
        if (!otaRaw)
        {
            throw std::runtime_error("Failed to read OTA file");
        }
        std::string otaStr(otaRaw->begin(), otaRaw->end());
        auto ota = parseOta(parseTdfFromString(otaStr));

        auto mapInfo = loadMap(mapName, ota, schemaIndex);
        std::unordered_set<std::string> requiredFeatureNames;
        for (const auto& f : mapInfo.features)
        {
            requiredFeatureNames.insert(f.second);
        }

        auto dataMaps = loadDefinitions(meshService, requiredFeatureNames);

        auto movementClassCollisionService = createMovementClassCollisionService(mapInfo.terrain, dataMaps.movementClassDatabase);

        GameSimulation simulation(std::move(mapInfo.terrain), mapInfo.surfaceMetal);

        simulation.unitDefinitions = std::move(dataMaps.unitDefinitions);
        simulation.weaponDefinitions = std::move(dataMaps.weaponDefinitions);
        simulation.movementClassDatabase = std::move(dataMaps.movementClassDatabase);
        simulation.movementClassCollisionService = std::move(movementClassCollisionService);
        simulation.unitModelDefinitions = dataMaps.modelDefinitions;
        simulation.unitScriptDefinitions = loadCobScripts(*sceneContext.vfs);
        simulation.featureDefinitions = std::move(dataMaps.featureDefinitions);
        simulation.featureNameIndex = std::move(dataMaps.featureNameIndex);

        for (const auto& [pos, featureName] : mapInfo.features)
        {
            auto featureId = simulation.tryGetFeatureDefinitionId(featureName).value();
            simulation.addFeature(featureId, pos.x, pos.y);
        }

        auto seedSeq = seedFromGameParameters(gameParameters);
        simulation.rng.seed(seedSeq);

        auto minimap = sceneContext.textureService->getMinimap(mapName);

        GameCameraState worldCameraState;

        std::optional<PlayerId> localPlayerId;

        auto playerCommandService = std::make_unique<PlayerCommandService>();

        std::array<std::optional<PlayerId>, 10> gamePlayers;
        std::vector<GameNetworkService::EndpointInfo> endpointInfos;

        for (Index i = 0; i < getSize(gameParameters.players); ++i)
        {
            const auto& params = gameParameters.players[i];
            if (params)
            {
                auto playerType = std::visit(IsComputerVisitor(), params->controller) ? GamePlayerType::Computer : GamePlayerType::Human;
                GamePlayerInfo gpi{params->name, playerType, params->color, GamePlayerStatus::Alive, params->side, params->metal, params->energy, params->metal, params->energy, params->metal, params->energy};
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
            std::move(dataMaps.gameMediaDatabase),
            worldCameraState,
            atlasInfo.textureAtlas,
            std::move(atlasInfo.teamTextureAtlases),
            std::move(simulation),
            std::move(mapInfo.terrainGraphics),
            std::move(dataMaps.builderGuisDatabase),
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

            auto worldStartPos = gameScene->getTerrain().topLeftCoordinateToWorld(SimVector(SimScalar(startPos.xPos), 0_ss, SimScalar(startPos.zPos)));
            worldStartPos.y = gameScene->getTerrain().getHeightAt(worldStartPos.x, worldStartPos.z);

            if (*gamePlayers[i] == *localPlayerId)
            {
                humanStartPos = worldStartPos;
            }

            const auto& sideData = getSideData(player->side);
            gameScene->spawnCompletedUnit(sideData.commander, *gamePlayers[i], worldStartPos);
        }

        if (!humanStartPos)
        {
            throw std::runtime_error("No human player!");
        }

        gameScene->setCameraPosition(Vector3f(simScalarToFloat(humanStartPos->x), 0.0f, simScalarToFloat(humanStartPos->z)));

        return gameScene;
    }


    LoadingScene::LoadMapResult LoadingScene::loadMap(const std::string& mapName, const OtaRecord& ota, unsigned int schemaIndex)
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
            SimScalar(tnt.getHeader().seaLevel));

        MapTerrainGraphics terrainGraphics(
            std::move(tileTextures),
            std::move(dataGrid));

        const auto& schema = ota.schemas.at(schemaIndex);

        auto featureNames = getFeatureNames(tnt);
        std::vector<std::pair<Point, std::string>> features;

        mapAttributes.forEachIndexed([&](auto c, const auto& e) {
            switch (e.feature)
            {
                case TntTileAttributes::FeatureNone:
                case TntTileAttributes::FeatureUnknown:
                case TntTileAttributes::FeatureVoid:
                    break;
                default:
                    features.emplace_back(Point(c.x, c.y), featureNames.at(e.feature));
            }
        });

        // add features from the OTA schema
        for (const auto& f : schema.features)
        {
            features.emplace_back(Point(f.xPos, f.zPos), f.featureName);
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


    const SideData& LoadingScene::getSideData(const std::string& side) const
    {
        auto it = sceneContext.sideData->find(side);
        if (it == sceneContext.sideData->end())
        {
            throw std::runtime_error("Missing side data for " + side);
        }

        return it->second;
    }

    void LoadingScene::loadFeatureMedia(MeshService& meshService, std::unordered_map<std::string, UnitModelDefinition>& modelDefinitions, GameMediaDatabase& gameMediaDatabase, const FeatureTdf& tdf)
    {
        FeatureMediaInfo f;

        f.world = tdf.world;
        f.description = tdf.description;
        f.category = tdf.category;

        if (!tdf.object.empty())
        {
            auto normalizedObjectName = toUpper(tdf.object);

            if (modelDefinitions.find(normalizedObjectName) == modelDefinitions.end())
            {
                auto meshInfo = meshService.loadProjectileMesh(normalizedObjectName);
                modelDefinitions.insert({normalizedObjectName, std::move(meshInfo.modelDefinition)});
                for (const auto& m : meshInfo.pieceMeshes)
                {
                    gameMediaDatabase.addUnitPieceMesh(normalizedObjectName, m.first, m.second);
                }
            }
            f.renderInfo = FeatureObjectInfo{normalizedObjectName};
        }
        else
        {
            FeatureSpriteInfo spriteInfo;
            spriteInfo.transparentAnimation = tdf.animTrans;
            spriteInfo.transparentShadow = tdf.shadTrans;
            if (!tdf.fileName.empty() && !tdf.seqName.empty())
            {
                spriteInfo.animation = sceneContext.textureService->getGafEntry("anims/" + tdf.fileName + ".GAF", tdf.seqName);
            }
            if (!spriteInfo.animation)
            {
                spriteInfo.animation = sceneContext.textureService->getDefaultSpriteSeries();
            }

            if (!tdf.fileName.empty() && !tdf.seqNameShad.empty())
            {
                // Some third-party features have broken shadow anim names (e.g. "empty"),
                // ignore them if they don't exist.
                spriteInfo.shadowAnimation = sceneContext.textureService->tryGetGafEntry("anims/" + tdf.fileName + ".GAF", tdf.seqNameShad);
            }
            f.renderInfo = std::move(spriteInfo);
        }

        f.seqNameReclamate = tdf.seqNameReclamate;

        f.seqNameBurn = tdf.seqNameBurn;
        f.seqNameBurnShad = tdf.seqNameBurnShad;

        f.seqNameDie = tdf.seqNameDie;

        gameMediaDatabase.addFeature(std::move(f));
    }

    void LoadingScene::loadFeature(MeshService& meshService, GameMediaDatabase& gameMediaDatabase, const std::unordered_map<std::string, FeatureTdf>& tdfs, DataMaps& dataMaps, const std::string& initialFeatureName)
    {
        auto nextId = dataMaps.featureDefinitions.getNextId();
        std::unordered_map<std::string, FeatureDefinitionId> openSet{{toUpper(initialFeatureName), nextId}};
        nextId = FeatureDefinitionId(nextId.value + 1);
        for (std::deque<std::string> featuresToLoad{{initialFeatureName}}; !featuresToLoad.empty(); featuresToLoad.pop_front())
        {
            const auto& featureName = featuresToLoad.front();

            const auto& tdf = tdfs.at(toUpper(featureName));

            FeatureDefinition f;

            f.name = featureName;

            f.footprintX = tdf.footprintX;
            f.footprintZ = tdf.footprintZ;
            f.height = SimScalar(tdf.height);

            f.reclaimable = tdf.reclaimable;
            f.autoreclaimable = tdf.autoreclaimable;
            if (!tdf.featureReclamate.empty())
            {
                f.featureReclamate = getFeatureId(nextId, dataMaps.featureNameIndex, featuresToLoad, openSet, tdf.featureReclamate);
            }
            f.metal = tdf.metal;
            f.energy = tdf.energy;

            f.flamable = tdf.flamable;
            if (!tdf.featureBurnt.empty())
            {
                f.featureBurnt = getFeatureId(nextId, dataMaps.featureNameIndex, featuresToLoad, openSet, tdf.featureBurnt);
            }
            f.burnMin = tdf.burnMin;
            f.burnMax = tdf.burnMax;
            f.sparkTime = tdf.sparkTime;
            f.spreadChance = tdf.spreadChance;
            f.burnWeapon = tdf.burnWeapon;

            f.geothermal = tdf.geothermal;

            f.hitDensity = tdf.hitDensity;

            f.reproduce = tdf.reproduce;
            f.reproduceArea = tdf.reproduceArea;

            f.noDisplayInfo = tdf.noDisplayInfo;

            f.permanent = tdf.permanent;

            f.blocking = tdf.blocking;

            f.indestructible = tdf.indestructible;
            f.damage = tdf.damage;
            if (!tdf.featureDead.empty())
            {
                f.featureDead = getFeatureId(nextId, dataMaps.featureNameIndex, featuresToLoad, openSet, tdf.featureDead);
            }

            auto id = dataMaps.featureDefinitions.insert(f);
            dataMaps.featureNameIndex.insert({toUpper(featureName), id});

            loadFeatureMedia(meshService, dataMaps.modelDefinitions, gameMediaDatabase, tdf);
        }
    }

    LoadingScene::DataMaps LoadingScene::loadDefinitions(MeshService& meshService, const std::unordered_set<std::string>& requiredFeatures)
    {
        DataMaps dataMaps;

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
                preloadSound(dataMaps.gameMediaDatabase, c.select1);
                preloadSound(dataMaps.gameMediaDatabase, c.unitComplete);
                preloadSound(dataMaps.gameMediaDatabase, c.activate);
                preloadSound(dataMaps.gameMediaDatabase, c.deactivate);
                preloadSound(dataMaps.gameMediaDatabase, c.ok1);
                preloadSound(dataMaps.gameMediaDatabase, c.arrived1);
                preloadSound(dataMaps.gameMediaDatabase, c.cant1);
                preloadSound(dataMaps.gameMediaDatabase, c.underAttack);
                preloadSound(dataMaps.gameMediaDatabase, c.build);
                preloadSound(dataMaps.gameMediaDatabase, c.repair);
                preloadSound(dataMaps.gameMediaDatabase, c.working);
                preloadSound(dataMaps.gameMediaDatabase, c.cloak);
                preloadSound(dataMaps.gameMediaDatabase, c.uncloak);
                preloadSound(dataMaps.gameMediaDatabase, c.capture);
                preloadSound(dataMaps.gameMediaDatabase, c.count5);
                preloadSound(dataMaps.gameMediaDatabase, c.count4);
                preloadSound(dataMaps.gameMediaDatabase, c.count3);
                preloadSound(dataMaps.gameMediaDatabase, c.count2);
                preloadSound(dataMaps.gameMediaDatabase, c.count1);
                preloadSound(dataMaps.gameMediaDatabase, c.count0);
                preloadSound(dataMaps.gameMediaDatabase, c.cancelDestruct);
                dataMaps.gameMediaDatabase.addSoundClass(s.first, std::move(s.second));
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
            auto classes = parseMoveInfoTdf(parseTdfFromString(movementString));
            for (auto& c : classes)
            {
                auto movementClassDefinition = parseMovementClassDefinition(c.second);
                dataMaps.movementClassDatabase.registerMovementClass(movementClassDefinition);
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
                    auto weaponDefinition = parseWeaponDefinition(pair.second);
                    auto weaponMediaInfo = parseWeaponMediaInfo(*sceneContext.palette, *sceneContext.guiPalette, pair.second);

                    preloadSound(dataMaps.gameMediaDatabase, weaponMediaInfo.soundStart);
                    preloadSound(dataMaps.gameMediaDatabase, weaponMediaInfo.soundHit);
                    preloadSound(dataMaps.gameMediaDatabase, weaponMediaInfo.soundWater);

                    if (auto modelRenderType = std::get_if<ProjectileRenderTypeModel>(&weaponMediaInfo.renderType); modelRenderType != nullptr)
                    {
                        auto meshInfo = meshService.loadProjectileMesh(modelRenderType->objectName);
                        dataMaps.modelDefinitions.insert({modelRenderType->objectName, std::move(meshInfo.modelDefinition)});
                        for (const auto& m : meshInfo.pieceMeshes)
                        {
                            dataMaps.gameMediaDatabase.addUnitPieceMesh(modelRenderType->objectName, m.first, m.second);
                        }
                    }

                    if (weaponMediaInfo.explosionAnim)
                    {
                        auto anim = sceneContext.textureService->getGafEntry("anims/" + weaponMediaInfo.explosionAnim->gafName + ".gaf", weaponMediaInfo.explosionAnim->animName);
                        dataMaps.gameMediaDatabase.addSpriteSeries(weaponMediaInfo.explosionAnim->gafName, weaponMediaInfo.explosionAnim->animName, anim);
                    }
                    if (weaponMediaInfo.waterExplosionAnim)
                    {
                        auto anim = sceneContext.textureService->getGafEntry("anims/" + weaponMediaInfo.waterExplosionAnim->gafName + ".gaf", weaponMediaInfo.waterExplosionAnim->animName);
                        dataMaps.gameMediaDatabase.addSpriteSeries(weaponMediaInfo.waterExplosionAnim->gafName, weaponMediaInfo.waterExplosionAnim->animName, anim);
                    }

                    dataMaps.gameMediaDatabase.addWeapon(toUpper(pair.first), std::move(weaponMediaInfo));

                    dataMaps.weaponDefinitions.insert({toUpper(pair.first), std::move(weaponDefinition)});
                }
            }
        }

        std::unordered_set<std::string> requiredFeaturesSet;
        for (const auto& f : requiredFeatures)
        {
            requiredFeaturesSet.insert(toUpper(f));
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

                auto unitDefinition = parseUnitDefinition(fbi, dataMaps.movementClassDatabase);
                dataMaps.unitDefinitions.insert({toUpper(fbi.unitName), std::move(unitDefinition)});

                // if it's a builder, also attempt to read its gui pages
                if (fbi.builder)
                {
                    auto guiPages = loadBuilderGui(fbi.unitName);
                    if (guiPages)
                    {
                        dataMaps.builderGuisDatabase.addBuilderGui(fbi.unitName, std::move(*guiPages));
                    }

                    // TODO: if no gui defined, attempt to build it dynamically?
                    // Need a database of download.tdf mappings first...
                }

                auto meshInfo = meshService.loadUnitMesh(fbi.objectName);
                dataMaps.modelDefinitions.insert({toUpper(fbi.objectName), std::move(meshInfo.modelDefinition)});
                for (const auto& m : meshInfo.pieceMeshes)
                {
                    dataMaps.gameMediaDatabase.addUnitPieceMesh(fbi.objectName, m.first, m.second);
                }

                dataMaps.gameMediaDatabase.addSelectionCollisionMesh(fbi.objectName, std::make_shared<CollisionMesh>(std::move(meshInfo.selectionMesh.collisionMesh)));
                dataMaps.gameMediaDatabase.addSelectionMesh(fbi.objectName, std::make_shared<GlMesh>(std::move(meshInfo.selectionMesh.visualMesh)));

                if (!fbi.corpse.empty())
                {
                    requiredFeaturesSet.insert(toUpper(fbi.corpse));
                }
            }
        }

        // read feature TDFs
        {
            auto files = sceneContext.vfs->getFileNamesRecursive("features", ".tdf");

            std::unordered_map<std::string, FeatureTdf> featureTdfs;

            for (const auto& name : files)
            {
                auto bytes = sceneContext.vfs->readFile("features/" + name);
                if (!bytes)
                {
                    throw std::runtime_error("Failed to read feature " + name);
                }

                std::string tdfString(bytes->data(), bytes->size());

                auto tdfRoot = parseTdfFromString(tdfString);
                for (const auto& e : tdfRoot.blocks)
                {
                    auto featureTdf = parseFeatureTdf(*e.second);
                    featureTdfs.insert({toUpper(e.first), featureTdf});
                }
            }

            // actually parse and load assets for features that we require
            for (const auto& featureName : requiredFeaturesSet)
            {
                loadFeature(meshService, dataMaps.gameMediaDatabase, featureTdfs, dataMaps, featureName);
            }
        }

        // preload smoke
        {
            auto anim = sceneContext.textureService->getGafEntry("anims/FX.GAF", "smoke 1");
            dataMaps.gameMediaDatabase.addSpriteSeries("FX", "smoke 1", anim);
            auto anim2 = sceneContext.textureService->getGafEntry("anims/FX.GAF", "smoke 2");
            dataMaps.gameMediaDatabase.addSpriteSeries("FX", "smoke 2", anim2);
        }

        // preload weapon sprites
        {
            auto anim = sceneContext.textureService->getGafEntry("anims/FX.GAF", "cannonshell");
            dataMaps.gameMediaDatabase.addSpriteSeries("FX", "cannonshell", anim);
        }
        {
            auto anim = sceneContext.textureService->getGafEntry("anims/FX.GAF", "plasmasm");
            dataMaps.gameMediaDatabase.addSpriteSeries("FX", "plasmasm", anim);
        }
        {
            auto anim = sceneContext.textureService->getGafEntry("anims/FX.GAF", "plasmamd");
            dataMaps.gameMediaDatabase.addSpriteSeries("FX", "plasmamd", anim);
        }
        {
            auto anim = sceneContext.textureService->getGafEntry("anims/FX.GAF", "ultrashell");
            dataMaps.gameMediaDatabase.addSpriteSeries("FX", "ultrashell", anim);
        }
        {
            auto anim = sceneContext.textureService->getGafEntry("anims/FX.GAF", "flamestream");
            dataMaps.gameMediaDatabase.addSpriteSeries("FX", "flamestream", anim);
        }

        return dataMaps;
    }

    void LoadingScene::preloadSound(GameMediaDatabase& meshDb, const std::optional<std::string>& soundName)
    {
        if (!soundName)
        {
            return;
        }

        preloadSound(meshDb, *soundName);
    }

    void LoadingScene::preloadSound(GameMediaDatabase& meshDb, const std::string& soundName)
    {
        auto sound = sceneContext.audioService->loadSound(soundName);
        if (!sound)
        {
            return; // sometimes sound categories name invalid sounds
        }

        meshDb.addSound(soundName, *sound);
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
