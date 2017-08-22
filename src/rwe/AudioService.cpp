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
        Mix_Chunk* sound;

        auto soundIter = soundBank.find(soundName);
        if (soundIter == soundBank.end())
        {
            auto bytes = fileSystem->readFile("sounds/" + soundName + ".WAV");
            if (!bytes)
            {
                throw std::runtime_error("Failed to load sound");
            }

            auto rwOps = sdlContext->rwFromConstMem(bytes->data(), bytes->size());
            auto loadedSound = sdlMixerContext->loadWavRw(rwOps.get());
            sound = loadedSound.get();
            soundBank[soundName] = std::move(loadedSound);
        }
        else
        {
            sound = soundIter->second.get();
        }

        sdlMixerContext->playChannel(-1, sound, -1);
    }
}
