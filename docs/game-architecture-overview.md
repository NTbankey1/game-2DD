# Game Architecture Overview (cập nhật 2026-07-06)

Dự án game endless runner viết bằng **C++20 + SDL3 + EnTT ECS**, kiến trúc 3-layer.

## Layers

```
main.cpp
└── game/          (gameplay logic — phụ thuộc engine, KHÔNG NGƯỢC LẠI)
    └── states/       PlayingState, PausedState, GameOverState (IScene)
    └── input/        InputMapper, InputCommand
    └── engine_impl/  MenuState
└── engine/        (framework — không biết gì về "jump", "coin", "cactus")
    └── scene/        GameStateMachine, IScene
    └── physics/      PhysicsSystem, CollisionSystem
    └── renderer/     RenderSystem, TextRenderer, BitmapFont
    └── platform/sdl3/SDLRenderer, SDLWindow, SDLInputDevice
    └── application/  Application (+ Textures/Drawing/Particles)
    └── camera/       Camera
    └── input/        IInputDevice, KeyEvent
└── core/          (primitive types — zero dependency)
    └── math/         Vec2, Rect, Math (Lerp, Clamp)
    └── events/       IEvent, EventBus (publish/subscribe)
    └── config/       ConfigTypes
```

## State Machine

```
Menu ──ENTER──▶ PlayingState ──ESC──▶ PausedState ──ESC──▶ PlayingState
                    │
                    ▼
              GameOverState ──ENTER──▶ PlayingState (fresh)
```

Mỗi state kế thừa `engine::IScene`:
- `OnEnter()` — khởi tạo registry, reset game data
- `Update(dt)` — input, logic, spawning
- `FixedUpdate(dt)` — physics + collision (fixed timestep 1/60s)
- `Render()` — draw to SDL_Renderer
- `OnExit()` — cleanup

## Game Systems (trong PlayingState)

Thứ tự update cố định:

| Step | System | Chức năng |
|------|--------|-----------|
| 1 | Collision detection | AABB overlap → set gameOver flag |
| 2 | Distance + Phase | Tính phase 1-5 dựa trên quãng đường |
| 3 | Jump | Coyote time (70ms), jump buffer (100ms), variable gravity (0.6x rise, 1.5x fall) |
| 4 | Player animation | Sprite sheet cycling (4 frame chân chạy), squash/stretch |
| 5 | Coin collection | AABB overlap → combo multiplier (10 × combo), particle |
| 6 | Obstacle spawning | Phase-aware: 1 loại → có short + double từ phase 4 |
| 7 | Scrolling | Dịch entity + destroy ra khỏi màn hình |
| 8 | Particle + popup update | Fade, swap-remove dead particles |
| 9 | Trail update | Afterimage ghost khi chạy trên đất |

## Rendering Pipeline (PlayingState)

```
1. Gradient sky (4px strip lerp)
2. Stars (twinkle + parallax 0.05x)
3. Clouds (5 lớp, parallax 0.2x-0.6x)
4. Mountains (3 lớp parallax: 0.08x, 0.18x, 0.29x)
5. Speed lines (phase 3+, blend alpha)
6. Particles (dust, coin burst, death explosion)
7. Trail afterimage (10 ghost, alpha fade)
8. ECS entities (RenderSystem)
9. Camera shake (random offset ±shakeAmount)
10. Score popups (+N floating text)
11. HUD bar (score, dist, coins, combo, phase, speed)
```

## Difficulty Phases

| Phase | Threshold | Đặc điểm |
|-------|-----------|----------|
| 1 | < 2000px | Obstacle cao 48px, interval dài |
| 2 | 2000px | Short obstacle 28px, coin đôi |
| 3 | 6000px | Speed lines, tốc độ tăng |
| 4 | 14000px | Double obstacle (cách 80-160px) |
| 5 | 25000px | Max speed, mọi pattern |

## File Structure hiện tại

```
src/
├── main.cpp
├── CMakeLists.txt
├── core/
│   ├── CMakeLists.txt
│   ├── core.hpp
│   ├── dummy.cpp
│   ├── config/ConfigTypes.hpp
│   ├── events/IEvent.hpp, EventBus.hpp
│   └── math/Math.hpp, Vec2.hpp, Rect.hpp
├── engine/
│   ├── CMakeLists.txt
│   ├── application/Application.hpp, Application.cpp
│   │                 ApplicationTextures.cpp
│   │                 ApplicationDrawing.cpp
│   │                 ApplicationParticles.cpp
│   │                 IApplication.hpp
│   ├── camera/Camera.hpp
│   ├── input/IInputDevice.hpp, KeyEvent.hpp
│   ├── physics/PhysicsSystem.hpp/.cpp
│   │           CollisionSystem.hpp/.cpp
│   │           PhysicsComponents.hpp
│   ├── platform/sdl3/SDLWindow.hpp/.cpp
│   │                  SDLRenderer.hpp/.cpp
│   │                  SDLInputDevice.hpp/.cpp
│   ├── renderer/RenderSystem.hpp/.cpp
│   │            RenderComponents.hpp
│   │            TextRenderer.hpp/.cpp
│   │            BitmapFont.hpp
│   │            IRenderer.hpp
│   └── scene/IScene.hpp
│             GameStateMachine.hpp/.cpp
└── game/
    ├── CMakeLists.txt
    ├── engine_impl/MenuState.hpp/.cpp
    ├── input/InputMapper.hpp/.cpp, InputCommand.hpp
    └── states/PlayingState.hpp/.cpp
               PausedState.hpp/.cpp
               GameOverState.hpp/.cpp
```

## Key Design Decisions

1. **IScene pattern** — thay switch-case, mỗi state là class riêng, thêm state không đụng code cũ.
2. **EnTT ECS** — Transform, Velocity, AABB là component; PlayerTag, GroundTag, CoinTag là identity marker.
3. **EventBus** — có cấu trúc, chưa active mạnh (CollisionSystem publish nhưng chưa ai subscribe). Kế hoạch: gắn score/particle/audio vào event cho Phase tiếp.
4. **Pixel art procedural** — CreatePlayerSheetTexture vẽ tay 4 frame 40×54 bằng SetPixel — có thể thay bằng PNG load sau mà không đổi code logic.
5. **Object pooling** — chưa triển khai, particle dùng linear array swap-remove.
