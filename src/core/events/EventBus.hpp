#pragma once

#include "IEvent.hpp"
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace core::events {

/// Type-erased listener handle (for RAII auto-unsubscribe)
struct ListenerHandle {
    std::type_index type = typeid(void);
    std::uint64_t id = 0;
    bool operator==(const ListenerHandle&) const = default;
};

namespace detail {
    inline std::uint64_t NextListenerId() {
        static std::uint64_t counter = 0;
        return counter++;
    }
}

/// Type-safe event bus. Listeners receive events via std::function callback.
/// Events are dispatched synchronously on the calling thread.
class EventBus {
public:
    template<typename E>
    using Callback = std::function<void(const E&)>;

    EventBus() = default;
    ~EventBus() = default;

    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;
    EventBus(EventBus&&) = default;
    EventBus& operator=(EventBus&&) = default;

    /// Subscribe to event type E. Returns a handle for unsubscribe.
    template<typename E>
    ListenerHandle Subscribe(Callback<E> callback) {
        static_assert(std::is_base_of_v<IEvent, E>, "E must derive from IEvent");
        auto& listeners = GetListeners(typeid(E));
        auto id = detail::NextListenerId();
        listeners[id] = [cb = std::move(callback)](const IEvent& e) {
            cb(static_cast<const E&>(e));
        };
        return {typeid(E), id};
    }

    /// Unsubscribe using handle
    void Unsubscribe(const ListenerHandle& handle) {
        auto it = m_listeners.find(handle.type);
        if (it != m_listeners.end()) {
            it->second.erase(handle.id);
        }
    }

    /// Publish event — all listeners receive it immediately.
    template<typename E>
    void Publish(const E& event) {
        static_assert(std::is_base_of_v<IEvent, E>, "E must derive from IEvent");
        auto it = m_listeners.find(typeid(E));
        if (it == m_listeners.end()) return;
        // Copy listeners map to allow unsubscribe during dispatch
        auto listeners_copy = it->second;
        for (const auto& [id, cb] : listeners_copy) {
            cb(event);
        }
    }

    /// Clear all listeners
    void Clear() {
        m_listeners.clear();
    }

private:
    using ListenerMap = std::unordered_map<std::uint64_t, std::function<void(const IEvent&)>>;

    ListenerMap& GetListeners(std::type_index type) {
        return m_listeners[type];
    }

    std::unordered_map<std::type_index, ListenerMap> m_listeners;
};

} // namespace core::events
