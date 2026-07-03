#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace core::config {

/// Generic named value for runtime-accessible configuration
struct ConfigValue {
    enum class Type { Integer, Float, String, Bool };
    Type type{};
    union {
        int64_t asInt{};
    };
    double asFloat{};
    std::string asString{};
    bool asBool{};

    ConfigValue() : type(Type::Integer), asInt(0) {}
    explicit ConfigValue(int64_t v) : type(Type::Integer), asInt(v) {}
    explicit ConfigValue(double v) : type(Type::Float), asFloat(v) {}
    explicit ConfigValue(std::string v) : type(Type::String), asString(std::move(v)) {}
    explicit ConfigValue(bool v) : type(Type::Bool), asBool(v) {}
};

using ConfigMap = std::unordered_map<std::string, ConfigValue>;

} // namespace core::config
