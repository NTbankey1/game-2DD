# Game Architecture Overview (cập nhật 2026-07-06)

Dự án 2D exploration game viết bằng **C++20 + SDL3 + SDL3_ttf + EnTT ECS**, kiến trúc 4-layer.

## Layers

```
main.cpp
└── game/          (gameplay logic — phụ thuộc engine, KHÔNG NGƯỢC LẠI)
    └── states/       ExplorationState, PausedState, GameOverState (IScene)
    └── player/       PlayerMovement, PlayerAnimationBindings
    └── input/        InputMapper, InputCommand
    └── item/         ItemComponent, ItemDatabase
    └── inventory/    InventorySystem
    └── interaction/  InteractionSystem
    └── dialogue/     DialogueSystem, DialogueData, DialogueEvents
    └── quest/        QuestSystem, QuestData
    └── enemy/        EnemyComponent
    └── combat/       CombatSystem, CombatComponent
    └── ai/           EnemyFSM, BossFSM
    └── checkpoint/   CheckpointSystem
    └── world/        PortalSystem, PortalComponent
    └── achievement/  AchievementSystem
    └── save/         GameSaveData
    └── ui/           InventoryUI, DialogueUI, QuestLogUI, MinimapUI
    └── engine_impl/  MenuState
└── engine/        (framework — không biết "player", "coin", "cactus")
    └── scene/        GameStateMachine, IScene
    └── physics/      PhysicsSystem, CollisionSystem
    └── renderer/     RenderSystem, TextRenderer (TTF), IRenderer
    └── animation/    AnimationController (generic, state machine)
    └── tilemap/      Tilemap, TilemapLoader, TilemapRenderer
    └── save/         SaveManager (JSON via nlohmann)
    └── application/  Application (+ Textures/Drawing/Particles)
    └── platform/sdl3/SDLRenderer, SDLWindow, SDLInputDevice
    └── camera/       Camera (follow, bounds, shake)
    └── input/        IInputDevice, KeyEvent
└── core/          (primitives — zero dependency)
    └── math/         Vec2, Rect, Math (Lerp, Clamp)
    └── events/       IEvent, EventBus (publish/subscribe)
    └── config/       ConfigTypes
```

## State Machine

```
Menu ──ENTER──▶ ExplorationState ──ESC──▶ PausedState ──ESC──▶ ExplorationState
                    │
       (HP=0)       │
                    ▼
              GameOverState ──ENTER──▶ Menu (clean) ──ENTER──▶ ExplorationState (fresh)
```

Mỗi state kế thừa `engine::IScene`:
- `OnEnter()` — khởi tạo registry, subscribe events
- `Update(dt)` — game logic (dialogue advance)
- `FixedUpdate(dt)` — physics (fixed timestep 1/60s)
- `Render()` — draw to SDL_Renderer với TTF font
- `OnExit()` — unsubscribe event handlers

## Game Systems (trong ExplorationState)

Thứ tự update cố định trong `Render()`:

| Step | System | Chức năng |
|------|--------|-----------|
| 1 | Player input | Horizontal movement (A/D), jump (SPACE), dash (SHIFT) |
| 2 | Physics | Gravity, velocity integration, coyote time (70ms), jump buffer (100ms) |
| 3 | Collision | AABB-vs-tilemap solid tiles, AABB-vs-ground, player-vs-obstacle |
| 4 | Player movement | Walk 280px/s, dash 600px/s (150ms, 500ms CD), wall-slide |
| 5 | Jump | Coyote 70ms + buffer 100ms + variable height (0.6x rise, 1.5x fall) |
| 6 | Player animation | Sprite sheet 4 frame cycling (15fps Idle/Run/Jump/Fall/WallSlide/Dash) |
| 7 | Coin collection | Proximity check + combo multiplier + particles |
| 8 | Interaction | E key near chest/NPC → dialogue/chest open |
| 9 | Dialogue | Linear dialogue system (4 node test) → auto-start quest |
| 10 | Enemy AI | Patrol/Chase/Attack FSM + Boss (2-phase, 10 HP) |
| 11 | Combat | Hitbox-Hurtbox overlap → damage + knockback + invuln timer (0.3s) |
| 12 | Checkpoint | Check activation on proximity → respawn position |
| 13 | Portal | Proximity teleport with particles |
| 14 | Achievements | Auto-subscribe QuestCompleted/EnemyDied events → unlock |
| 15 | Particle + popup | Fade, swap-remove dead particles |
| 16 | Save/Load | F5 save (JSON), F9 load — score/HP/position |
| 17 | Trail | Afterimage ghost khi chạy trên đất |

## Rendering Pipeline

```
0. Tilemap background layer (colored rects per tile ID, culled to viewport)
1. Gradient sky (4px strip lerp)
2. Stars (100 twinkle + parallax 0.05x)
3. Clouds (5 lớp, parallax 0.2x-0.6x)
4. Mountains (3 lớp parallax: 0.08x, 0.18x, 0.29x)
5. Speed lines (phase 3+, blended alpha)
6. Particles (dust, coin burst, death explosion, portal)
7. Trail afterimage (10 ghosts, alpha fade, frame-correct)
8. ECS entities (RenderSystem via camera)
9. Camera shake (random offset ±shakeAmount, decay 0.85x)
10. Score popups (+N floating text, fades up)
11. Minimap (discovery-based, player X marker)
12. Dialogue box (dark panel + speaker name + text + E prompt)
13. HUD bar (dark + accent line + font sizes 14-18)
    ├── HP hearts (♥/♡)
    ├── Controls hint
    ├── Coin counter
    ├── Inventory panel (right)
    └── Quest log panel (left)
14. Interaction prompt ([E] Open Chest / [E] Talk — center-aligned, pulsing)
```

## World Map — test_world.json

World: **100 × 23 tiles** (32px/tile → **3200 × 736px**)

| Zone | X range | Tiles | Nội dung |
|------|---------|-------|----------|
| Safe | 0-25 | 0-10 | NPC (x=450), Portal (x=300), Chest (x=650) |
| Gap | 25-45 | 10-15 | Hố platforming, có Portal nhảy qua |
| Enemy | 45-65 | 15-22 | Slime (x=1600), Rock barriers |
| Checkpoint | ~47 | ~16 | x=1500, respawn nếu chết |
| Boss arena | 85-100 | 20-23 | Boss (x=2900, 10 HP, 2-phase) |

## File Structure hiện tại

```
src/
├── main.cpp
├── CMakeLists.txt
├── core/
│   ├── CMakeLists.txt, core.hpp, dummy.cpp
│   ├── config/ConfigTypes.hpp
│   ├── events/IEvent.hpp, EventBus.hpp
│   └── math/Math.hpp, Vec2.hpp, Rect.hpp
├── engine/
│   ├── CMakeLists.txt
│   ├── animation/AnimationController.hpp/.cpp
│   ├── application/Application.hpp, IApplication.hpp
│   │   ├── Application.cpp
│   │   ├── ApplicationTextures.cpp
│   │   ├── ApplicationDrawing.cpp
│   │   └── ApplicationParticles.cpp
│   ├── camera/Camera.hpp
│   ├── input/IInputDevice.hpp, KeyEvent.hpp
│   ├── physics/PhysicsSystem.hpp/.cpp
│   │   ├── CollisionSystem.hpp/.cpp
│   │   └── PhysicsComponents.hpp
│   ├── platform/sdl3/SDLWindow.hpp/.cpp
│   │   ├── SDLRenderer.hpp/.cpp
│   │   └── SDLInputDevice.hpp/.cpp
│   ├── renderer/RenderSystem.hpp/.cpp
│   │   ├── RenderComponents.hpp
│   │   ├── TextRenderer.hpp/.cpp (SDL3_ttf)
│   │   ├── BitmapFont.hpp (kept for reference)
│   │   └── IRenderer.hpp
│   ├── save/SaveManager.hpp/.cpp
│   ├── scene/IScene.hpp, GameStateMachine.hpp/.cpp
│   └── tilemap/Tilemap.hpp, TilemapLoader.cpp, TilemapRenderer.cpp
├── game/
│   ├── CMakeLists.txt
│   ├── achievement/AchievementSystem.hpp/.cpp
│   ├── ai/EnemyFSM.hpp/.cpp, BossFSM.hpp/.cpp
│   ├── checkpoint/CheckpointSystem.hpp/.cpp
│   ├── combat/CombatComponent.hpp, CombatSystem.hpp/.cpp
│   ├── dialogue/DialogueData.hpp, DialogueEvents.hpp
│   │   └── DialogueSystem.hpp/.cpp
│   ├── enemy/EnemyComponent.hpp
│   ├── engine_impl/MenuState.hpp/.cpp
│   ├── input/InputMapper.hpp/.cpp, InputCommand.hpp
│   ├── interaction/InteractionSystem.hpp/.cpp
│   ├── inventory/InventorySystem.hpp/.cpp
│   ├── item/ItemComponent.hpp, ItemDatabase.hpp/.cpp
│   ├── npc/NPCComponent.hpp
│   ├── player/PlayerMovement.hpp/.cpp
│   │   └── PlayerAnimationBindings.hpp/.cpp
│   ├── quest/QuestData.hpp, QuestSystem.hpp/.cpp
│   ├── save/GameSaveData.hpp/.cpp
│   ├── states/ExplorationState.hpp/.cpp
│   │   ├── PausedState.hpp/.cpp
│   │   └── GameOverState.hpp/.cpp
│   ├── ui/InventoryUI.hpp/.cpp
│   │   ├── DialogueUI.hpp/.cpp
│   │   ├── QuestLogUI.hpp/.cpp
│   │   └── MinimapUI.hpp/.cpp
│   └── world/PortalComponent.hpp, PortalSystem.hpp/.cpp
```

## Dependencies

| Library | Dùng cho |
|---------|----------|
| **SDL3** | Windowing, rendering, input |
| **SDL3_ttf** | TrueType font rendering (3 sizes: 20/22/30) |
| **EnTT** | ECS registry, components, views |
| **spdlog** | Logging |
| **fmt** | String formatting |
| **nlohmann/json** | Tilemap JSON loading + save file serialization |
| **Catch2** | Unit testing (51 assertions, 25 test cases) |

## Key Design Decisions

1. **IScene pattern** — mỗi state là class riêng, thêm state không đụng code cũ. Application::Run() chỉ còn ~60 dòng.
2. **EnTT ECS** — Transform, Velocity, AABB, PlayerTag, CoinTag, EnemyComponent.
3. **EventBus** — InventoryChangedEvent, EnemyDiedEvent, QuestEvents, DialogueEvents, DamageDealtEvent có subscriber thật.
4. **SDL3_ttf** — thay bitmap font 8×13, font Hack Nerd Font Regular, 3 sizes, anti-aliased.
5. **Pixel art procedural** — CreatePlayerSheetTexture vẽ tay 4 frame 40×54 bằng SetPixel — có thể thay bằng PNG load sau.
6. **Object pooling** — Particles dùng linear array swap-remove (80 slots).
