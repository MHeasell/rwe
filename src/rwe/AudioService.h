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
    public:
        using Sound = Mix_Chunk;
        using SoundHandle = std::shared_ptr<Sound>;

        class LoopToken
        {
        private:
            AudioService* audioService;
            int channel;
            SoundHandle sound;
        public:
            LoopToken();
            LoopToken(AudioService* audioService, int channel, const SoundHandle& sound);
            ~LoopToken();
            LoopToken(const LoopToken&) = delete;
            LoopToken& operator=(const LoopToken&) = delete;
            LoopToken(LoopToken&& other) noexcept;
            LoopToken& operator=(LoopToken&& other) noexcept;
            boost::optional<const SoundHandle&> getSound();
        };

    private:
        SdlContext* sdlContext;
        SdlMixerContext* sdlMixerContext;
        AbstractVirtualFileSystem* fileSystem;
        std::unordered_map<std::string, std::shared_ptr<Sound>> soundBank;

    public:
        AudioService(SdlContext* sdlContext, SdlMixerContext* sdlMixerContext, AbstractVirtualFileSystem* fileSystem);

        LoopToken loopSound(const SoundHandle& sound);

        void playSound(const SoundHandle& sound);

        boost::optional<SoundHandle> loadSound(const std::string& soundName);

    private:
        void haltChannel(int channel);
    };
}

#endif
