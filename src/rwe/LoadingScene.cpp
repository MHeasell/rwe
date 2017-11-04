#include <boost/interprocess/streams/bufferstream.hpp>
#include <rwe/ota.h>
#include <rwe/tdf.h>
#include <rwe/tnt/TntArchive.h>
#include <rwe/ui/UiLabel.h>
#include "LoadingScene.h"

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
        CursorService* cursor,
        GraphicsContext* graphics,
        MapFeatureService* featureService,
        const ColorPalette* palette,
        SceneManager* sceneManager,
        SdlContext* sdl,
        const std::unordered_map<std::string, SideData>* sideData,
        AudioService::LoopToken&& bgm,
        GameParameters gameParameters)
        : vfs(vfs),
          textureService(textureService),
          cursor(cursor),
          graphics(graphics),
          featureService(featureService),
          palette(palette),
          sceneManager(sceneManager),
          sdl(sdl),
          sideData(sideData),
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
        panel->render(context);
        cursor->render(context);
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

        auto terrain = createMapTerrain(mapName, ota, schemaIndex);

        CabinetCamera camera(640.0f, 480.0f);
        camera.setPosition(Vector3f(0.0f, 0.0f, 0.0f));

        auto meshService = MeshService::createMeshService(vfs, graphics, palette);

        std::vector<AttribMapping> unitTextureShaderAttribs{
            AttribMapping{"position", 0},
            AttribMapping{"texCoord", 1}};

        std::vector<AttribMapping> unitColorShaderAttribs{
            AttribMapping{"position", 0},
            AttribMapping{"color", 1}};

        std::vector<AttribMapping> selectBoxShaderAttribs{
            AttribMapping{"position", 0}};

        SharedShaderProgramHandle unitTextureShader{loadShader("shaders/unitTexture.vert", "shaders/unitTexture.frag", unitTextureShaderAttribs)};
        SharedShaderProgramHandle unitColorShader{loadShader("shaders/unitColor.vert", "shaders/unitColor.frag", unitColorShaderAttribs)};
        SharedShaderProgramHandle selectBoxShader{loadShader("shaders/selectBox.vert", "shaders/selectBox.frag", selectBoxShaderAttribs)};

        auto unitDatabase = createUnitDatabase();

        auto gameScene = std::make_unique<GameScene>(
            textureService,
            cursor,
            sdl,
            std::move(meshService),
            std::move(camera),
            std::move(terrain),
            std::move(unitTextureShader),
            std::move(unitColorShader),
            std::move(selectBoxShader),
            std::move(unitDatabase));

        const auto& schema = ota.schemas.at(schemaIndex);

        Vector3f humanStartPos;

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

            // TODO: detect which player is the human
            if (i == 0)
            {
                humanStartPos = worldStartPos;
            }

            const auto& sideData = getSideData(player->side);
            gameScene->spawnUnit(sideData.commander, worldStartPos);
        }

        gameScene->setCameraPosition(Vector3f(humanStartPos.x, 0.0f, humanStartPos.z));

        return gameScene;
    }

    MapTerrain LoadingScene::createMapTerrain(const std::string& mapName, const OtaRecord& ota, unsigned int schemaIndex)
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
            std::move(heightGrid));

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
                        Vector3f pos = computeFeaturePosition(terrain, featureTemplate, x, y);
                        auto feature = createFeature(pos, featureTemplate);
                        terrain.getFeatures().push_back(feature);
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
            terrain.getFeatures().push_back(feature);
        }

        return terrain;
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

        std::vector<Color> textureBuffer(textureWidth * textureHeight);

        std::vector<SharedTextureHandle> textureHandles;

        // read the tile graphics into textures
        {
            unsigned int tileCount = 0;
            tnt.readTiles([this, &tileCount, &textureBuffer, &textureHandles](const char* tile) {
                if (tileCount == tilesPerTexture)
                {
                    SharedTextureHandle handle(graphics->createTexture(textureWidth, textureHeight, textureBuffer));
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
                        textureBuffer[(textureY * textureWidth) + textureX] = (*palette)[index];
                    }
                }

                tileCount += 1;
            });
        }
        textureHandles.emplace_back(graphics->createTexture(textureWidth, textureHeight, textureBuffer));

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
            f.shadowAnimation = textureService->getGafEntry("anims/" + definition.fileName + ".GAF", definition.seqNameShad);
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

    ShaderProgramHandle
    LoadingScene::loadShader(const std::string& vertexShaderName, const std::string& fragmentShaderName, const std::vector<AttribMapping>& attribs)
    {
        auto vertexShaderSource = slurpFile(vertexShaderName);
        auto vertexShader = graphics->compileVertexShader(vertexShaderSource);

        auto fragmentShaderSource = slurpFile(fragmentShaderName);
        auto fragmentShader = graphics->compileFragmentShader(fragmentShaderSource);

        return graphics->linkShaderProgram(vertexShader.get(), fragmentShader.get(), attribs);
    }

    std::string LoadingScene::slurpFile(const std::string& filename)
    {
        std::ifstream inFile(filename, std::ios::binary);
        std::stringstream strStream;
        strStream << inFile.rdbuf();
        return strStream.str();
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
}
