#ifndef RWE_SCENECONTEXT_H
#define RWE_SCENECONTEXT_H

#include <rwe/AudioService.h>
#include <rwe/CursorService.h>
#include <rwe/MapFeatureService.h>
#include <rwe/SceneManager.h>
#include <rwe/SideData.h>
#include <rwe/TextureService.h>
#include <rwe/ViewportService.h>
#include <rwe/vfs/AbstractVirtualFileSystem.h>

namespace rwe
{
    class SceneContext
    {
    public:
        SdlContext* const sdl;
        ViewportService* const viewportService;
        GraphicsContext* const graphics;
        TextureService* const textureService;
        AudioService* const audioService;
        CursorService* const cursor;
        ShaderService* const shaders;
        AbstractVirtualFileSystem* const vfs;
        const ColorPalette* const palette;
        const ColorPalette* const guiPalette;
        SceneManager* const sceneManager;
        const std::unordered_map<std::string, SideData>* const sideData;

        SceneContext(
            SdlContext* const sdl,
            ViewportService* const viewportService,
            GraphicsContext* const graphics,
            TextureService* const textureService,
            AudioService* const audioService,
            CursorService* const cursor,
            ShaderService* const shaders,
            AbstractVirtualFileSystem* const vfs,
            const ColorPalette* const palette,
            const ColorPalette* const guiPalette,
            SceneManager* const sceneManager,
            const std::unordered_map<std::string, SideData>* const sideData)
            : sdl(sdl),
              viewportService(viewportService),
              graphics(graphics),
              textureService(textureService),
              audioService(audioService),
              cursor(cursor),
              shaders(shaders),
              vfs(vfs),
              palette(palette),
              guiPalette(guiPalette),
              sceneManager(sceneManager),
              sideData(sideData)
        {
        }
    };
}

#endif
