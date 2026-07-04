---
title: Memory & Performance Strategy
date: 2026-07-04
tags:
  - architecture
  - performance
  - memory
aliases:
  - Performance
  - Memory Strategy
---

# Memory & Performance Strategy

%% Memory management patterns và performance strategy cho 2D endless runner %%

## Golden Rule

> ==**Zero allocation in the gameplay hot path.**==
> Mọi allocation phải xảy ra ở init phase. Update loop chỉ dùng stack và pre-allocated pools.

---

## Object Pool ^object-pool

**Dùng cho:** Obstacles, coins, particles, UI elements.

```cpp
template<typename T>
class ObjectPool {
    static constexpr size_t MAX_OBSTACLES = 128;
    static constexpr size_t MAX_PARTICLES = 256;

    struct Slot {
        alignas(alignof(T)) std::byte storage[sizeof(T)];
        bool active{false};
    };
    std::array<Slot, MAX_OBSTACLES> m_pool;
    std::vector<size_t> m_freeList;  // Indices of free slots
};
```

| Pool | Size | Tại sao |
|------|------|---------|
| Obstacles | 128 | Tối đa obstacle trên màn hình |
| Coins | 64 | Tối đa coin |
| Particles | 256 | Death effect, coin sparkle |
| UI Elements | 32 | Menus, buttons |

> [!tip] Object Pool Hiệu Quả Khi
> - Object thường xuyên được tạo/hủy (obstacles)
> - Kích thước object không quá lớn
> - Số lượng object có upper bound

---

## ECS Cache Friendliness

```cpp
// EnTT stores components in contiguous arrays
// Iterating over components = sequential memory access
registry.view<Transform, Velocity>().each(
    [](auto& transform, auto& velocity) {
        // Cache-friendly: Transform and Velocity packed together
        transform.pos += velocity.v * dt;
    }
);
```

> [!info] ECS layout vs OOP layout
> - OOP: `Entity*[]` → random access → cache miss
> - ECS: `Transform[]` + `Velocity[]` → linear access → cache hit

---

## Fail Fast ^fail-fast

Những gì crash ngay lúc startup, không phải lúc gameplay:

```cpp
// GOOD: Crash at startup
auto texture = SDL_CreateTextureFromSurface(renderer, surface);
ASSERT(texture != nullptr, "Failed to load player texture");

// BAD: Silent null → crash later at random point
// auto texture = SDL_CreateTextureFromSurface(renderer, surface);
// if (!texture) return; // ← silent skip, bug hides
```

| Check | Vị trí | Kết quả |
|-------|--------|---------|
| Asset load | Init | Crash với message rõ |
| Config validation | Init | Crash nếu config sai |
| Pool overflow | Runtime (debug) | `dev_assert` crash |
| Null handle | Runtime | `ASSERT` crash |
| Out of memory | Runtime | `std::bad_alloc` |

---

## Hot Path Analysis ^hot-path

### Profiling Priority (từ hot nhất → cold nhất)

| Path | Frequency | Budget | Kỹ thuật |
|------|-----------|--------|----------|
| RenderSystem::update | Every frame | < 8ms | Batching, culling |
| PhysicsSystem::update | 60/sec | < 2ms | Fixed timestep |
| CollisionSystem::update | 60/sec | < 2ms | Broad → narrow |
| Input polling | Every frame | < 0.1ms | Event-driven |

### What NOT to do in hot path

```cpp
// ❌ NO heap allocation
// auto obstacle = std::make_unique<Obstacle>();
// ❌ NO virtual calls per object (use ECS)
// ❌ NO dynamic_cast
// ❌ NO std::string creation
// ❌ NO file I/O
// ✅ ObjectPool::acquire()
// ✅ ECS view iteration
// ✅ Direct component access
```

---

## Memory Budget

| Area | Budget | Notes |
|------|--------|-------|
| ECS Components | ~2 MB | ~10,000 entities × ~20 components |
| Textures (procedural) | ~1 MB | SDL3 GPU textures |
| Audio buffers | ~4 MB | Phase 2 |
| Object Pools | ~1 MB | Obstacles, particles, UI |
| Stack | ~1 MB | Configurable in CMake |
| **Total** | **~9 MB** | Well under 16MB target |

---

## Compiler Optimizations

```cmake
# CMakePresets.json
target_compile_options(game PRIVATE
    -O3                    # Maximum optimization
    -march=native          # CPU-specific optimizations
    -flto                  # Link-time optimization
    -fno-exceptions        # No exceptions (release)
    -fno-rtti              # No RTTI (release)
)
```

> [!warning] Profile-Guided Optimization
> PGO chưa active. Kích hoạt khi profile cho thấy cần thêm 5-10%.

---

## Related Notes
- [[Runtime Flow]] — understand the hot path
- [[Gameplay Systems]] — which systems are on the hot path
- [[Testing Strategy]] — performance regression tests
- [[Architecture Pitfalls#premature-optimization]] — when not to optimize

^memory-performance
