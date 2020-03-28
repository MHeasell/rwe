#pragma once

#include <functional>
#include <memory>
#include <rwe/SdlContextManager.h>
#include <rwe/observable/Subject.h>
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
            std::optional<std::reference_wrapper<const SoundHandle>> getSound();
        };

    private:
        SdlContext* sdlContext;
        SdlMixerContext* sdlMixerContext;
        AbstractVirtualFileSystem* fileSystem;
        std::unordered_map<std::string, std::shared_ptr<Sound>> soundBank;
        Subject<int> channelFinished;
        std::function<void(int)> channelFinishedCallback = std::bind(&Subject<int>::next, &channelFinished, std::placeholders::_1);

    public:
        AudioService(SdlContext* sdlContext, SdlMixerContext* sdlMixerContext, AbstractVirtualFileSystem* fileSystem);
        AudioService(const AudioService&) = delete;
        AudioService(const AudioService&&) = delete;
        AudioService& operator=(const AudioService&) = delete;
        AudioService& operator=(AudioService&&) = delete;

        LoopToken loopSound(const SoundHandle& sound);

        int playSound(const SoundHandle& sound);

        std::optional<SoundHandle> loadSound(const std::string& soundName);

        void reserveChannels(unsigned int count);

        void playSoundIfFree(const SoundHandle& sound, unsigned int channel);

        void setVolume(int channel, int volume);

        Observable<int>& getChannelFinished();

    private:
        void haltChannel(int channel);
    };
}
