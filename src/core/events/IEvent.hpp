#pragma once

namespace core::events {

/// Base interface for all event types.
/// Event types MUST be:
///   - Default-constructible (for pooling)
///   - Copy-constructible  (for dispatch queue)
///   - Serializable (via cereal — Phase 4+)
struct IEvent {
    virtual ~IEvent() = default;
};

} // namespace core::events
