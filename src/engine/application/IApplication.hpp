#pragma once
#include "core/core.hpp"

namespace engine {
class IApplication {
public:
    virtual ~IApplication() = default;
    virtual bool Initialize() = 0;
    virtual void Run() = 0;
    virtual void Shutdown() = 0;
    virtual float GetFrameTime() const = 0;
};
} // namespace engine
