#pragma once
#include "core/core.hpp"

namespace engine {
class IAudioDevice {
public:
    virtual ~IAudioDevice() = default;
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void PlaySound(const char* name, float volume = 1.0f) = 0;
    virtual void SetVolume(float volume) = 0;
};
} // namespace engine
