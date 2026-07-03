#pragma once
#include "core/core.hpp"

namespace engine {
class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
};
} // namespace engine
