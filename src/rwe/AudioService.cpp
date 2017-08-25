#include "AudioService.h"

namespace rwe
{
    AudioService::AudioService(
        SdlContext* sdlContext,
        SdlMixerContext* sdlMixerContext,
        AbstractVirtualFileSystem* fileSystem)
        : sdlContext(sdlContext),
          sdlMixerContext(sdlMixerContext),
          fileSystem(fileSystem)
    {
    }

    void AudioService::loopSound(const std::string& soundName)
    {
        auto sound = loadSound(soundName);
        if (!sound)
        {
            throw std::runtime_error("Failed to load sound");
        }

        sdlMixerContext->playChannel(-1, *sound, -1);
    }

    void AudioService::playSound(const std::string& soundName)
    {
        auto sound = loadSound(soundName);
        if (!sound)
        {
            throw std::runtime_error("Failed to load sound");
        }

        sdlMixerContext->playChannel(-1, *sound, 0);
    }

    boost::optional<Mix_Chunk*> AudioService::loadSound(const std::string& soundName)
    {

        auto soundIter = soundBank.find(soundName);
        if (soundIter != soundBank.end())
        {
            return soundIter->second.get();
        }


        auto bytes = fileSystem->readFile("sounds/" + soundName + ".WAV");
        if (!bytes)
        {
            return boost::none;
        }

        auto rwOps = sdlContext->rwFromConstMem(bytes->data(), bytes->size());
        auto loadedSound = sdlMixerContext->loadWavRw(rwOps.get());
        auto sound = loadedSound.get();
        soundBank[soundName] = std::move(loadedSound);

        return sound;
    }
}
