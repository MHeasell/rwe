#pragma once

#include <functional>
#include <memory>
#include <rwe/observable/Subject.h>
#include <rwe/sdl/SdlContext.h>
#include <rwe/sdl/SdlMixerContext.h>
#include <rwe/vfs/AbstractVirtualFileSystem.h>
#include <unordered_map>
#include <vector>

namespace rwe
{
    class AudioService
    {
    public:
        using Sound = MIX_Audio;
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

        // Track pool: maps channel indices to MIX_Track pointers.
        // Tracks 0..reservedCount-1 are "reserved" (used by playSoundIfFree).
        std::vector<SdlMixerContext::TrackPtr> tracks;
        unsigned int reservedCount{0};

        // Default gain applied to sounds on load (equivalent to old MIX_MAX_VOLUME/4)
        static constexpr float defaultGain = 0.25f;

    public:
        AudioService(SdlContext* sdlContext, SdlMixerContext* sdlMixerContext, AbstractVirtualFileSystem* fileSystem);
        AudioService(const AudioService&) = delete;
        AudioService(const AudioService&&) = delete;
        AudioService& operator=(const AudioService&) = delete;
        AudioService& operator=(AudioService&&) = delete;

        void allocateTracks(unsigned int count);

        LoopToken loopSound(const SoundHandle& sound);

        int playSound(const SoundHandle& sound);

        std::optional<SoundHandle> loadSound(const std::string& soundName);

        void reserveChannels(unsigned int count);

        void playSoundIfFree(const SoundHandle& sound, unsigned int channel);

        void setVolume(int channel, int volume);

        Observable<int>& getChannelFinished();

    private:
        void haltChannel(int channel);
        int findFreeTrack();
        void setupTrackCallback(int trackIndex);
    };
}
