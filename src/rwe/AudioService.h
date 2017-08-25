#ifndef RWE_AUDIOSERVICE_H
#define RWE_AUDIOSERVICE_H

#include <memory>
#include <rwe/SdlContextManager.h>
#include <rwe/vfs/AbstractVirtualFileSystem.h>
#include <unordered_map>

namespace rwe
{
    class AudioService
    {
    private:
        SdlContext* sdlContext;
        SdlMixerContext* sdlMixerContext;
        AbstractVirtualFileSystem* fileSystem;
        std::unordered_map<std::string, std::unique_ptr<Mix_Chunk, SdlMixerContext::MixChunkDeleter>> soundBank;

    public:
        AudioService(SdlContext* sdlContext, SdlMixerContext* sdlMixerContext, AbstractVirtualFileSystem* fileSystem);

        void loopSound(const std::string& soundName);

        void playSound(const std::string& soundName);

    private:
        boost::optional<Mix_Chunk*> loadSound(const std::string& soundName);

    };
}

#endif
