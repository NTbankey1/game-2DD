#pragma once
#include "core/core.hpp"
#include <memory>

namespace engine {
template<typename T>
class IAssetLoader {
public:
    virtual ~IAssetLoader() = default;
    virtual std::shared_ptr<T> Load(const std::string& path) = 0;
    virtual void Unload(const std::string& path) = 0;
    virtual bool IsLoaded(const std::string& path) const = 0;
};
} // namespace engine
