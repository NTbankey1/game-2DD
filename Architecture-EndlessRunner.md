# KIẾN TRÚC DỰ ÁN: 2D ENDLESS RUNNER ENGINE

**Vai trò tài liệu:** Bản thiết kế kiến trúc tổng thể (Architecture Design Document) trước khi viết bất kỳ dòng code nào.
**Ngôn ngữ:** C++20 · **Build:** CMake · **Compiler:** GCC/Clang/MSVC

---

## 1. TỔNG QUAN HỆ THỐNG

### 1.1 Tầm nhìn (Vision)

Đây không phải là "một game runner nhỏ viết cho vui". Đây là một **engine mini chuyên biệt cho thể loại 2D action/runner**, trong đó "Endless Runner" chỉ là **game đầu tiên** được dựng trên engine đó. Toàn bộ quyết định kiến trúc phải trả lời được câu hỏi:

> "Nếu 2 năm nữa tôi cần thêm boss, multiplayer leaderboard, level editor, và game thứ hai dùng chung engine — tôi có phải viết lại từ đầu không?"

Nếu câu trả lời là "có", thiết kế đó bị loại ngay từ vòng suy nghĩ.

### 1.2 Nguyên tắc thiết kế cốt lõi

| Nguyên tắc | Ý nghĩa thực tế |
|---|---|
| **Engine/Game Separation** | `engine/` không được include hay biết bất kỳ thứ gì trong `game/`. Engine là thư viện, game là client của thư viện đó. |
| **Data-Oriented khi cần, OOP khi hợp lý** | Dùng ECS (EnTT) cho GameObject/Component thay vì cây kế thừa sâu. |
| **Composition over Inheritance** | Component-based, không có `class FlyingSpikeBoss : public Obstacle : public Entity : public GameObject`. |
| **Everything is replaceable** | Renderer, AudioBackend, InputBackend đều là interface — SDL3 chỉ là 1 implementation. |
| **Fail fast, log everything** | spdlog ở mọi layer quan trọng, assertion dày trong Debug build. |
| **No premature networking, but leave the door open** | Không code online ngay, nhưng Event Bus + Save System phải serializable từ đầu (dùng cereal), vì đó chính là nền cho leaderboard/replay sau này. |

### 1.3 Phạm vi Phase 1 (MVP) vs Kiến trúc đầy đủ

Điểm quan trọng: **MVP không có nghĩa là kiến trúc rút gọn.** MVP nghĩa là ít *content* hơn (1 map, 1 nhân vật, 1 chế độ chơi), nhưng bộ khung (skeleton) module, interface, Event Bus, Resource Manager, Scene System đều được dựng đầy đủ ngay từ Phase 1 để không phải refactor kiến trúc về sau — chỉ cần *thêm* implementation mới (map thứ 2, nhân vật thứ 2...) mà không đụng vào core.

---

## 2. KIẾN TRÚC TỔNG THỂ

Áp dụng **Clean Architecture** biến thể cho game, chia thành 4 vòng tròn phụ thuộc (dependency chỉ được đi vào trong, không được đi ra ngoài):

```
┌─────────────────────────────────────────────────────────┐
│  Layer 4: PLATFORM / THIRD-PARTY                          │
│  SDL3, OpenGL, GLM, spdlog, nlohmann/json, EnTT, cereal   │
└───────────────────────┬───────────────────────────────────┘
                         │ implements interfaces from
┌───────────────────────▼───────────────────────────────────┐
│  Layer 3: ENGINE (Infrastructure + Adapters)               │
│  Renderer, AudioDevice, InputDevice, FileSystem,           │
│  ResourceManager, SceneManager, ECS wrapper, EventBus      │
└───────────────────────┬───────────────────────────────────┘
                         │ used by (via interface, DIP)
┌───────────────────────▼───────────────────────────────────┐
│  Layer 2: GAMEPLAY SYSTEMS (Application/Use-Case layer)    │
│  PlayerController, JumpSystem, ObstacleSpawner,             │
│  ScoreSystem, DifficultyScaler, SaveSystem, AchievementSys  │
└───────────────────────┬───────────────────────────────────┘
                         │ operates on
┌───────────────────────▼───────────────────────────────────┐
│  Layer 1: CORE / DOMAIN                                    │
│  Entity, Component definitions (POD), Events (POD),        │
│  Math types, GameConfig, pure logic không phụ thuộc gì cả  │
└─────────────────────────────────────────────────────────────┘
```

**Quy tắc phụ thuộc (Dependency Rule):**
- Layer 1 (Core) không `#include` bất cứ thứ gì từ Layer 2, 3, 4. Đây là pure C++/STL/GLM (chỉ struct toán học).
- Layer 2 (Gameplay) không được include SDL3, OpenGL trực tiếp — chỉ include interface trừu tượng do Engine cung cấp (`IRenderer`, `IAudioDevice`...).
- Layer 3 (Engine) là nơi duy nhất được phép include thư viện thứ ba cụ thể (SDL3.h, GL.h...).
- **UI không phụ thuộc Physics**: UI chỉ lắng nghe Event (`ScoreChanged`, `GamePaused`) qua Event Bus, không bao giờ gọi trực tiếp vào `PhysicsSystem`.
- **Game Logic không phụ thuộc Renderer**: `JumpSystem` chỉ sửa `Transform`/`Velocity` component; việc vẽ player thế nào là việc của `RenderSystem` đọc component đó ra vẽ, hai bên không biết nhau.

### 2.1 Vì sao không dùng kiến trúc "1 file main.cpp khổng lồ" hay Singleton God-Class?

- **Không mở rộng được**: thêm 1 obstacle mới sẽ đụng vào 10 chỗ khác nhau (compile toàn bộ lại, dễ xung đột merge).
- **Không test được**: logic dính chặt vào SDL3 render loop thì không unit-test được `JumpSystem` mà không mở cửa sổ.
- **Không tái sử dụng được**: game thứ hai dùng chung engine gần như không thể nếu code không phân lớp.

---

## 3. SƠ ĐỒ MODULE

```
                         ┌───────────────┐
                         │  Application  │  (entry point, main loop, game states)
                         └───────┬───────┘
                 ┌───────────────┼────────────────┐
                 ▼               ▼                ▼
          ┌───────────┐   ┌────────────┐   ┌─────────────┐
          │  Engine   │   │    Game    │   │    Editor   │ (tương lai, optional)
          └─────┬─────┘   └─────┬──────┘   └─────────────┘
    ┌────────────┼────────────┐ │
    ▼            ▼            ▼ ▼
┌────────┐  ┌─────────┐  ┌──────────┐  ┌───────────┐
│ Window │  │Renderer │  │  Audio   │  │  Input    │
└────────┘  └─────────┘  └──────────┘  └───────────┘
    │            │            │              │
    └────────────┴─────┬──────┴──────────────┘
                        ▼
                ┌───────────────┐
                │ ResourceMgr /  │
                │ AssetManager   │
                └───────┬────────┘
                        ▼
        ┌───────────────────────────────┐
        │  Scene / SceneManager          │
        │  (chứa Registry ECS - EnTT)    │
        └───────────┬───────────────────┘
                     ▼
      ┌───────────────────────────────────┐
      │   Systems (Gameplay Layer)          │
      │  Physics, Collision, Animation,      │
      │  Particle, Camera, AI, TileMap       │
      └───────────────┬───────────────────┘
                       ▼
              ┌────────────────┐
              │   Event Bus     │◄──── UI, SaveSystem,
              └────────────────┘       Achievement, Debug HUD
                       │
      ┌────────────────┼────────────────┐
      ▼                ▼                ▼
 ┌─────────┐    ┌─────────────┐   ┌────────────┐
 │SaveSystem│    │Achievement  │   │Leaderboard │
 └─────────┘    │  System     │   │ (stub nay, │
                 └─────────────┘   │  online mai)│
                                    └────────────┘
```

### 3.1 Vai trò từng khối

- **Application**: khởi tạo Window, Engine subsystems, vòng lặp `Update/Render`, quản lý `GameStateMachine` (Menu/Playing/Paused/GameOver).
- **Engine**: cung cấp *khả năng* (rendering, audio, input, resource loading) dưới dạng interface trừu tượng, độc lập thể loại game.
- **Game**: chứa *nội dung* và *luật chơi* cụ thể của Endless Runner (Player, Obstacle, Coin, DifficultyCurve...).
- **Editor** (tương lai): công cụ ngoài runtime, dùng lại Engine layer để chỉnh map/level mà không cần chạy toàn bộ game loop.

---

## 4. DEPENDENCY GRAPH (Chi tiết cấp module)

```
Core            ← không phụ thuộc gì (chỉ STL + GLM)
Events (Core)   ← Core
Utilities       ← Core
Config          ← Core, nlohmann/json

Window          ← Core, SDL3
Renderer        ← Core, Window, SDL3/OpenGL, GLM
Audio           ← Core, SDL3
Input           ← Core, SDL3, Events

ResourceManager ← Core, Utilities, stb_image, stb_truetype, Renderer(iface), Audio(iface)
AssetManager    ← ResourceManager

ECS (EnTT wrap) ← Core
Scene           ← ECS, Events
SceneManager    ← Scene, ResourceManager

Physics         ← Core, ECS, Events
Collision       ← Physics, ECS
Animation       ← ECS, ResourceManager
Particles       ← ECS, Renderer(iface)
Camera          ← ECS, GLM
TileMap         ← ECS, ResourceManager, Collision

AI              ← ECS, Events            (dùng cho boss sau này)
UI              ← Renderer(iface), Events, Fonts
Fonts           ← ResourceManager, stb_truetype

SaveSystem      ← Core, cereal, Events
Achievement     ← Events, SaveSystem
Leaderboard     ← Events, SaveSystem, Networking(iface, stub)
Networking      ← Core (chỉ interface rỗng ở giai đoạn 1)
Replay          ← Events, SaveSystem, ECS

Debug           ← toàn bộ layer trên (chỉ dùng trong Debug build, tách biệt bằng #ifdef/CMake option)

Game (Player, Obstacle, ObstacleSpawner, ScoreSystem, DifficultyScaler)
                ← Engine layer (qua interface), Core

Application     ← Engine, Game, (Editor optional)
Editor          ← Engine (KHÔNG phụ thuộc Game trực tiếp — Editor thao tác trên Scene/ECS chung)
```

**Nhận xét kiến trúc quan trọng:**
- `Game` **không bao giờ** xuất hiện ở vế trái của `Engine`. Điều này đảm bảo Engine có thể build & test độc lập, và có thể tái sử dụng cho game thứ 2.
- `Networking` là module rỗng (interface only) từ Phase 1 — để `Leaderboard`/`Replay` sau này chỉ cần cắm implementation thật (ví dụ REST client) vào mà không đổi API gọi từ Gameplay layer.

---

## 5. LUỒNG HOẠT ĐỘNG CỦA GAME (Runtime Flow)

### 5.1 Main Loop (Fixed Timestep + Interpolation)

```
Application::Run()
 ├─ Init: Window → Renderer → Audio → Input → ResourceManager → SceneManager
 ├─ LoadScene(MenuScene)
 └─ while (running)
     ├─ Input.PollEvents()               // SDL3 events → Engine Input events → EventBus
     ├─ accumulator += deltaTime
     ├─ while (accumulator >= FIXED_DT)  // physics/logic cố định 60Hz, tách khỏi FPS hiển thị
     │    ├─ CurrentGameState->FixedUpdate(FIXED_DT)
     │    │    ├─ InputSystem
     │    │    ├─ PhysicsSystem (gravity, velocity integrate)
     │    │    ├─ CollisionSystem → phát Event (PlayerDead, CoinCollected...)
     │    │    ├─ ObstacleSpawnSystem
     │    │    ├─ DifficultyScalingSystem
     │    │    ├─ ScoreSystem
     │    │    └─ AnimationSystem (state machine tick)
     │    └─ accumulator -= FIXED_DT
     ├─ CurrentGameState->Update(deltaTime)   // camera follow, particle spawn timers, UI
     ├─ EventBus.Dispatch()                    // xử lý toàn bộ event đã queue trong frame
     ├─ Renderer.BeginFrame()
     │    ├─ RenderSystem (parallax → tilemap → entities → particles → UI)
     └─ Renderer.EndFrame() / Present()
```

### 5.2 Vì sao Fixed Timestep?

- Vật lý nhảy/trượt của runner cực nhạy với delta time không ổn định → dễ "nhảy xuyên tường" ở máy yếu/khoẻ khác nhau nếu dùng variable timestep thuần.
- Fixed timestep cho physics + interpolation cho render là chuẩn công nghiệp (Glenn Fiedler's "Fix Your Timestep").

### 5.3 Luồng State Machine của Game

```
   ┌────────┐   Start    ┌─────────┐  Collision  ┌───────────┐
   │  Menu   ├───────────►│ Playing ├────────────►│ GameOver   │
   └───┬────┘             └────┬────┘             └─────┬─────┘
       │                       │ Pause                   │ Restart
       │                       ▼                          │
       │                 ┌──────────┐                     │
       │                 │  Paused  │                     │
       │                 └────┬─────┘                     │
       │      Resume          │                            │
       └───────────────────────┴────────────────────────────┘
```
Triển khai bằng **State Pattern** (`IGameState` interface: `OnEnter/OnExit/Update/FixedUpdate/Render`), quản lý bởi `GameStateMachine` (dạng Stack, để Pause chỉ push thêm state chứ không hủy state Playing bên dưới → giữ nguyên toàn bộ entity khi resume).

### 5.4 Luồng dữ liệu Event-driven (ví dụ va chạm)

```
CollisionSystem phát hiện Player chạm Obstacle
   → EventBus.Publish(PlayerDeadEvent{score, coins})
        ├─ AudioSystem lắng nghe → phát SFX death
        ├─ ParticleSystem lắng nghe → spawn hiệu ứng vỡ
        ├─ ScoreSystem lắng nghe → chốt điểm cuối
        ├─ SaveSystem lắng nghe → ghi high score nếu phá kỷ lục
        ├─ AchievementSystem lắng nghe → kiểm tra điều kiện mở khoá
        └─ GameStateMachine lắng nghe → chuyển sang GameOverState
```
→ Đây chính là lý do Event Bus là xương sống: **các hệ thống không gọi trực tiếp nhau**, giảm coupling gần như tuyệt đối.

---

## 6. CẤU TRÚC THƯ MỤC

```
EndlessRunnerEngine/
├── CMakeLists.txt                  # root build script
├── cmake/                          # CMake modules dùng chung
│   ├── CompilerWarnings.cmake
│   ├── Sanitizers.cmake
│   ├── StaticAnalyzers.cmake
│   ├── Dependencies.cmake          # FetchContent / find_package cho vendor libs
│   └── Packaging.cmake             # CPack config
│
├── assets/                         # asset RAW, không build (textures, audio nguồn)
│   ├── textures/
│   ├── audio/
│   ├── fonts/
│   └── maps/
│
├── resources/                      # asset đã build/optimize để ship (copy vào build output)
│
├── src/
│   ├── core/                       # Layer 1 — pure domain, không phụ thuộc gì
│   │   ├── math/
│   │   ├── ecs/                    # wrapper mỏng quanh EnTT (Entity, ComponentRegistry alias)
│   │   ├── events/                 # định nghĩa Event POD + EventBus interface
│   │   └── config/
│   │
│   ├── engine/                     # Layer 3 — infrastructure/adapters
│   │   ├── application/
│   │   ├── window/
│   │   ├── renderer/
│   │   │   ├── backend_sdl/
│   │   │   └── backend_gl/         # optional, bật bằng CMake option
│   │   ├── audio/
│   │   ├── input/
│   │   ├── resource/               # ResourceManager, AssetManager
│   │   ├── scene/                  # Scene, SceneManager
│   │   ├── ui/
│   │   ├── camera/
│   │   ├── animation/
│   │   ├── particles/
│   │   ├── physics/
│   │   ├── collision/
│   │   ├── tilemap/
│   │   ├── ai/
│   │   ├── save/
│   │   ├── achievement/
│   │   ├── leaderboard/
│   │   ├── networking/             # interface stub cho tương lai
│   │   ├── replay/
│   │   └── debug/                  # debug draw, profiler overlay, console
│   │
│   ├── game/                       # Layer 2 — Endless Runner content cụ thể
│   │   ├── player/
│   │   ├── obstacles/
│   │   ├── coins/
│   │   ├── scoring/
│   │   ├── difficulty/
│   │   ├── states/                 # MenuState, PlayingState, PausedState, GameOverState
│   │   └── config/                 # cân bằng game (jump force, gravity, spawn rate...)
│   │
│   └── main.cpp
│
├── tools/
│   ├── editor/                     # level editor (tương lai)
│   └── asset_pipeline/             # script convert/nén asset
│
├── tests/
│   ├── unit/                       # Catch2/GTest cho Core + Systems thuần logic
│   ├── integration/                # test kết hợp nhiều system qua ECS thật
│   └── benchmark/                  # Google Benchmark cho hot path (collision, spawn)
│
├── vendor/                         # third-party không có trên package manager, hoặc pin version
├── scripts/                        # build.sh, format.sh, ci scripts
└── docs/
    ├── Architecture.md
    ├── Patterns.md
    ├── Assets.md
    ├── Gameplay.md
    ├── Roadmap.md
    ├── API.md
    ├── Contribution.md
    └── CodingStandard.md
```

**Giải thích lựa chọn:**
- `core/` tách khỏi `engine/` dù nhiều dự án gộp chung — lý do: `core/ecs`, `core/events`, `core/math` phải compile được **không cần SDL3**, cho phép unit test cực nhanh (không mở cửa sổ, không cần GPU) và cho phép `tools/asset_pipeline` dùng lại mà không kéo theo cả renderer.
- `game/` tách biệt hoàn toàn `engine/` để về sau **game thứ hai** (ví dụ platformer) chỉ cần tạo `src/game2/` mới, dùng lại 100% `engine/`.
- `resources/` (build output) tách khỏi `assets/` (nguồn) để tránh commit file build vào git, và để `asset_pipeline` có chỗ ghi ra.

---

## 7. DANH SÁCH THƯ VIỆN & LÝ DO CHỌN

| Thư viện | Vai trò | Lý do chọn | Thay thế đã cân nhắc |
|---|---|---|---|
| **SDL3** | Window, Input, Audio, Renderer 2D | Cross-platform (Win/Linux/Mac), ổn định, ít abstraction leak, SDL3 đã có Renderer 2D tăng tốc GPU tích hợp sẵn nên không bắt buộc phải tự viết OpenGL cho MVP. Cộng đồng lớn, license zlib. | GLFW (chỉ window/input, không có audio/renderer, phải ghép 3 lib) |
| **OpenGL (optional, qua GLAD)** | Renderer nâng cao khi cần custom shader (particle nâng cao, shader effect) | Không bắt buộc ở Phase MVP nhưng để ngỏ vì SDL3 Renderer không cho viết shader tuỳ ý. Bật qua CMake option `ENABLE_GL_BACKEND`. | bgfx (mạnh nhưng nặng, overkill cho 2D) |
| **GLM** | Toán vector/matrix (vec2, mat4 cho camera) | Header-only, cú pháp giống GLSL, chuẩn công nghiệp cho game C++. | Eigen (thiên về khoa học/ma trận lớn, nặng hơn) |
| **spdlog** | Logging | Nhanh (async), format qua fmt, mức log linh hoạt (trace/debug/info/warn/error), ghi file + console cùng lúc. | Tự viết logger (tốn thời gian, ít tính năng) |
| **fmt** | String formatting | spdlog dùng sẵn fmt bên dưới, đồng bộ luôn cho code base, tránh `sprintf`/`stringstream` C-style. | std::format (mới, một số compiler/STL implementation chưa ổn định bằng fmt) |
| **nlohmann/json** | Đọc config, level data, localization | API rất trực quan (`json["key"]`), single header, được dùng cực rộng rãi để debug/edit bằng tay dễ dàng. | RapidJSON (nhanh hơn nhưng API khó dùng hơn nhiều cho config người đọc được) |
| **cereal** | Serialization cho Save System, Replay System | Header-only, hỗ trợ binary + JSON + XML cùng API, tích hợp tốt với struct C++ thường (không cần macro nặng như Protobuf). | Protobuf (overkill, cần build step riêng, phù hợp hơn cho networking sau này) |
| **EnTT** | ECS core | Rất nhanh (sparse set), header-only, không có "engine đi kèm" nên không đụng độ với Renderer riêng của mình, cộng đồng game dùng rộng rãi (Minecraft, nhiều game indie). | Tự viết ECS (tốn thời gian, khó tối ưu bằng EnTT đã được benchmark kỹ) |
| **Catch2** | Unit / Integration test | Cú pháp BDD-style dễ đọc (`SECTION`), single header option, tích hợp CTest mượt. | GoogleTest (cũng tốt, chọn 1 trong 2 tuỳ team quen tay; tài liệu sẽ dùng Catch2 làm mặc định) |
| **Google Benchmark** | Đo hiệu năng hot path | Chuẩn công nghiệp để benchmark CollisionSystem, ObjectPool, so sánh trước/sau tối ưu. | Tự đo bằng `chrono` (thiếu warm-up, statistical rigor) |
| **stb_image / stb_truetype** | Load texture PNG/JPG, render font từ file .ttf | Public domain, single header, không cần build hệ thống phức tạp như FreeType cho nhu cầu 2D UI đơn giản. | FreeType (mạnh hơn nhưng nặng, cần build riêng — chỉ cân nhắc nếu UI cần font phức tạp/đa ngôn ngữ nặng) |

**Nguyên tắc quản lý dependency:** dùng CMake `FetchContent`/`find_package` với version pin rõ ràng trong `cmake/Dependencies.cmake`, ưu tiên package đã có trên vcpkg/Conan để CI build nhanh và tái lập được.

---

## 8. DESIGN PATTERN ÁP DỤNG

| Pattern | Áp dụng ở đâu | Vì sao dùng | Nếu không dùng thì sao | Ưu điểm | Nhược điểm |
|---|---|---|---|---|---|
| **State** | `GameStateMachine` (Menu/Playing/Paused/GameOver), `PlayerAnimationState` (Idle/Run/Jump/Slide) | Hành vi thay đổi hoàn toàn theo trạng thái; tránh `if (state == X) {...} else if...` khổng lồ trong Update(). | Update() phình to, mỗi state mới phải sửa toàn bộ hàm cũ → vi phạm OCP. | Thêm state mới không đụng code cũ; dễ test từng state độc lập. | Thêm 1 lớp trừu tượng, hơi "nặng" nếu chỉ có 2 state đơn giản. |
| **Factory / Abstract Factory** | `ObstacleFactory`, `RendererFactory` (chọn backend SDL/GL lúc khởi tạo) | Tạo obstacle/renderer mà code gọi không cần biết class cụ thể → dễ thêm loại mới. | ObstacleSpawnSystem phải `new SpikeObstacle()/new BirdObstacle()` rải rác khắp nơi, khó mở rộng loại mới. | Tập trung logic tạo object; dễ đăng ký loại mới qua config/data-driven. | Thêm 1 tầng gián tiếp, cần đăng ký (registration) đúng lúc khởi động. |
| **Builder** | `SceneBuilder` (dựng scene từ file JSON: tilemap + spawn points + entities ban đầu) | Scene có nhiều bước dựng tuỳ chọn (có/không có background layer thứ n, có/không boss) — Builder tách quá trình dựng phức tạp thành các bước rõ ràng. | Constructor Scene nhận 15 tham số optional → cực khó đọc/maintain. | Code dựng scene đọc như một kịch bản tuần tự, dễ test từng bước. | Thừa nếu Scene chỉ có 2-3 field đơn giản. |
| **Strategy** | `IDifficultyStrategy` (Linear/Exponential/Curve theo thời gian chơi), `IInputStrategy` (Keyboard/Gamepad/Touch) | Thuật toán tăng độ khó hoặc input source cần đổi runtime hoặc theo config mà không sửa `DifficultyScalingSystem`. | Logic tăng khó hard-code cứng, muốn thử nghiệm balance mới phải sửa và build lại core system. | Đổi chiến lược runtime (A/B test độ khó), designer chỉnh không cần lập trình viên. | Cần định nghĩa interface rõ trước — chi phí thiết kế ban đầu cao hơn hard-code. |
| **Command** | `InputCommand` (Jump/Slide/Pause) tách khỏi phím vật lý | Cho phép rebind phím, và là nền tảng bắt buộc cho **Replay System** (ghi lại chuỗi Command theo frame để phát lại). | Không thể làm replay chuẩn xác nếu chỉ đọc raw key state; rebind phím phải sửa code cứng. | Ghi log input dễ dàng (replay, demo tự động, undo trong Editor). | Thêm một lớp gián tiếp giữa phím bấm và hành động. |
| **Observer / Event Bus** | Toàn bộ giao tiếp liên-module (xem mục 11) | Là xương sống giảm coupling giữa Gameplay/Audio/UI/Save/Achievement. | Các system gọi thẳng lẫn nhau → mạng lưới phụ thuộc chằng chịt, sửa 1 chỗ vỡ 10 chỗ. | Module hoàn toàn không biết nhau, thêm listener mới không sửa publisher. | Luồng thực thi khó trace bằng mắt (cần tool debug event log). |
| **Component (ECS)** | Toàn bộ GameObject (Player, Obstacle, Coin...) qua EnTT | Đối tượng game runner có tổ hợp thuộc tính đa dạng (có Physics + không AI, có AI + không Physics cho background prop...) → tránh cây kế thừa (diamond problem). | Cây kế thừa `Obstacle → FlyingObstacle → FlyingSpikeObstacle...` phình to, khó tái tổ hợp hành vi. | Composition linh hoạt, hiệu năng cao (cache-friendly với EnTT sparse set). | Tư duy ECS cần thời gian làm quen, debug entity qua ID thay vì object trực quan. |
| **Flyweight** | Texture/Font/Sound trong `ResourceManager` (chia sẻ 1 instance giữa nhiều entity) | Hàng trăm obstacle cùng loại share chung 1 texture; không load lại từng cái. | Tốn bộ nhớ/băng thông load lại asset trùng lặp, GC áp lực. | Giảm bộ nhớ đáng kể, load nhanh hơn nhờ cache. | Cần quản lý vòng đời cẩn thận (ai giữ shared_ptr, khi nào unload). |
| **Object Pool** | `ParticlePool`, `ObstaclePool`, `EventPool` (tránh new/delete liên tục mỗi frame) | Runner spawn/destroy obstacle & particle liên tục — cấp phát động mỗi frame gây fragmentation và GC spike (dù C++ không có GC, `new/delete` liên tục vẫn chậm và phân mảnh heap). | Giật lag định kỳ do malloc/free liên tục, đặc biệt rõ trên mobile/thiết bị yếu. | Hiệu năng ổn định, không cấp phát runtime trong hot loop. | Cần quản lý kích thước pool hợp lý, phức tạp hơn code "tạo rồi xoá" đơn giản. |
| **Service Locator** | Truy cập `AudioService`, `ResourceManager` từ nơi không tiện Dependency Injection sâu (ví dụ trong callback UI) | Dùng **hạn chế**, chỉ cho các service toàn cục thực sự (Audio, Resource) — không dùng cho gameplay logic. | Nếu không có gì thay thế, code sẽ truyền tham chiếu qua 5-6 tầng hàm ("parameter drilling"), rất khó đọc. | Giảm truyền tham số dây chuyền qua nhiều lớp gọi. | Nếu lạm dụng sẽ thành "Singleton trá hình", che giấu dependency thật, khó test. **→ Team phải review kỹ mọi lần thêm service mới vào Locator.** |
| **Dependency Injection** | `Application` khởi tạo và "tiêm" `IRenderer`, `IAudioDevice`, `IInputDevice` vào `Engine`/`SceneManager` qua constructor | Cho phép mock renderer/audio khi unit test Gameplay systems mà không cần SDL3 thật. | Systems tự `new SDLRenderer()` bên trong → không test được, không đổi backend được (VD: headless test trên CI). | Testability cao, đổi implementation không sửa code gọi (OCP). | Constructor có nhiều tham số hơn, cần 1 nơi "root composition" (thường ở `Application`) chịu trách nhiệm wiring. |
| **Prototype** | Clone entity template từ level data (VD: spawn 1 "spike cluster" đã định nghĩa sẵn cấu trúc) | Tạo nhanh nhiều entity giống nhau có biến thể nhỏ (vị trí) mà không phải build lại từ đầu mỗi lần. | ObstacleFactory phải build lại toàn bộ component set mỗi lần spawn, chậm hơn và dễ thiếu sót component. | Spawn nhanh, nhất quán với entity mẫu định nghĩa trong Editor/JSON. | Cần cẩn thận deep-copy đúng, tránh chia sẻ nhầm state (VD 2 obstacle share chung 1 Transform pointer). |
| **Facade** | `AudioFacade` (che giấu chi tiết SDL_mixer/SDL3 Audio, chỉ lộ `PlaySfx()/PlayMusic()`), `RenderFacade` cho gameplay dùng | Gameplay code chỉ cần API đơn giản, không cần biết chi tiết buffer/channel/mixing bên dưới. | Gameplay code lẫn lộn chi tiết low-level SDL API, khó đọc, khó đổi backend. | API gọn, che giấu độ phức tạp, dễ dùng cho lập trình viên gameplay. | Có thể giới hạn khả năng truy cập tính năng nâng cao nếu Facade thiết kế quá hẹp — cần để lộ "escape hatch" khi cần. |
| **Mediator** | `EventBus` cũng đóng vai trò Mediator giữa UI ↔ Gameplay ↔ Audio (thay vì UI gọi trực tiếp PauseSystem) | Giảm liên lạc trực tiếp N-N giữa các module UI/Gameplay. | UI Button `OnClick` gọi thẳng vào `PhysicsSystem::Pause()` → vi phạm rule "UI không phụ thuộc Physics". | Tập trung điều phối, dễ thêm bên thứ 3 lắng nghe cùng sự kiện pause (VD Analytics sau này). | Nếu Mediator quá tải logic sẽ thành "God Object" — cần giữ EventBus chỉ làm việc *chuyển tiếp*, không chứa business logic. |
| **Composite** | `UI Widget Tree` (Panel chứa Button/Label/Panel con), `TileMap Layer` (nhiều layer composite thành 1 map) | UI/Map có cấu trúc cây tự nhiên, cần xử lý đồng nhất node lá và node chứa con (render, layout, update). | Code UI phải phân biệt thủ công "đây là container hay leaf" ở mọi nơi duyệt cây → rối. | Duyệt cây thống nhất qua interface chung; dễ thêm loại widget/layer mới. | Overhead nhỏ nếu cây quá nông (2-3 UI đơn giản không cần Composite đầy đủ). |
| **Chain of Responsibility** | `InputProcessingChain` (UI nhận input trước → nếu không xử lý thì chuyển xuống Gameplay), `CollisionResolverChain` (loại va chạm khác nhau xử lý theo thứ tự ưu tiên) | Input cần "ai xử lý trước được quyền chặn" (VD dialog UI mở thì Gameplay không nhận input jump). | Logic if/else kiểm tra "UI có đang mở không" rải rác khắp InputSystem. | Thêm handler mới (VD thêm lớp Editor overlay) không sửa chain cũ. | Debug thứ tự chain sai có thể gây input "biến mất" khó trace nếu không log rõ. |

**Nguyên tắc chống lạm dụng Singleton:** Chỉ 1 thứ duy nhất được phép gần giống Singleton là `Logger` (spdlog global logger instance, vốn dĩ bản chất log là global theo thiết kế của chính spdlog). Mọi service khác (ResourceManager, EventBus, AudioDevice...) được sở hữu bởi `Application`/`Engine` và **truyền xuống qua tham chiếu/DI**, không qua `GetInstance()` toàn cục.

---

## 9. SOLID TRONG DỰ ÁN

| Nguyên tắc | Vi phạm điển hình (nếu làm ẩu) | Cách áp dụng đúng trong dự án |
|---|---|---|
| **SRP** (Single Responsibility) | 1 class `Player` vừa xử lý input, vừa vật lý, vừa render, vừa âm thanh, vừa save score. | Tách thành `PlayerInputComponent` (data) + `JumpSystem` + `PhysicsSystem` + `RenderSystem` + `AudioSystem` — mỗi system chỉ có 1 lý do để thay đổi. VD: đổi công thức nhảy chỉ sửa `JumpSystem`, không đụng render. |
| **OCP** (Open/Closed) | Thêm obstacle mới phải sửa `switch(type)` trong `CollisionSystem`. | `IObstacleBehavior` interface + `ObstacleFactory` đăng ký loại mới qua config data-driven — thêm obstacle mới = thêm 1 file mới + 1 dòng đăng ký, không sửa `CollisionSystem`. |
| **LSP** (Liskov Substitution) | `class FlyingObstacle : public Obstacle` override `OnCollide()` ném exception vì bay không "va chạm kiểu thường" → phá vỡ hợp đồng lớp cha. | Với ECS, LSP áp dụng ở tầng interface: mọi `IRenderer` implementation (SDL backend, GL backend) phải thoả cùng hợp đồng `DrawSprite()/Present()` — code gọi `IRenderer&` không cần biết/không được ngạc nhiên bởi backend cụ thể. |
| **ISP** (Interface Segregation) | 1 interface `IEngineService` khổng lồ có cả `PlaySound()`, `DrawSprite()`, `SaveGame()` — class nào implement cũng phải "giả vờ" hỗ trợ hết. | Tách riêng `IAudioDevice`, `IRenderer`, `ISaveStorage`... — `ScoreSystem` chỉ phụ thuộc `ISaveStorage`, không kéo theo cả `IRenderer` mà nó không dùng. |
| **DIP** (Dependency Inversion) | `PhysicsSystem` include thẳng `<SDL3/SDL.h>` để lấy delta time từ SDL timer. | `PhysicsSystem` chỉ phụ thuộc abstraction `IClock`/tham số `deltaTime` truyền vào — `Application` (composition root) mới là nơi biết SDL3 tồn tại. Điều này cũng chính là nền tảng để Layer 2 (Gameplay) build/test được mà không cần SDL3 thật (dùng `MockRenderer`, `MockClock` trong test). |

---

## 10. GAMEPLAY SYSTEMS (Chi tiết)

Mỗi hệ thống dưới đây được thiết kế như **1 System độc lập trong ECS**, chỉ thao tác trên component qua Registry, giao tiếp ra ngoài qua Event Bus.

- **Player**: không phải 1 class, mà là 1 Entity mang các Component: `Transform`, `Velocity`, `Collider`, `PlayerTag`, `AnimationState`, `InputBinding`.
- **Jump**: `JumpSystem` đọc `InputCommand::Jump` (qua Command pattern) → nếu `IsGrounded`, set `Velocity.y = -JumpForce`, chuyển `AnimationState → Jumping`, publish `PlayerJumpEvent`. Hỗ trợ **coyote time** và **jump buffering** (chuẩn thiết kế platformer/runner tốt) — cấu hình qua `game/config`.
- **Gravity**: là 1 phần của `PhysicsSystem`, áp dụng gia tốc không đổi lên `Velocity.y` mỗi FixedUpdate, có `TerminalVelocity` cấu hình để tránh rơi quá nhanh khi map dài.
- **Collision**: `CollisionSystem` dùng AABB (đơn giản, đủ cho runner 2D) hoặc mở rộng SAT nếu cần obstacle nghiêng; tách `CollisionDetection` (tìm cặp va chạm) khỏi `CollisionResolution` (quyết định hậu quả: chết, nảy lại, nhặt coin) để 2 phần này test độc lập nhau.
- **Obstacle Spawn**: `ObstacleSpawnSystem` dùng Strategy pattern để chọn pattern spawn (theo khoảng cách đã chạy, không theo thời gian thực để né phụ thuộc framerate), lấy obstacle từ `ObstacleFactory` + `ObjectPool`.
- **Difficulty Scaling**: `IDifficultyStrategy` (Strategy pattern) nhận `distanceTravelled`/`elapsedTime`, trả về `speedMultiplier`, `spawnRateMultiplier` — designer chỉnh trong file JSON, không cần sửa code.
- **Coin**: Entity nhẹ với `Collider` + `CoinTag`, khi va chạm Player → publish `CoinCollectedEvent`, trả entity về Pool.
- **Score**: `ScoreSystem` lắng nghe `DistanceUpdatedEvent`/`CoinCollectedEvent`, cộng dồn, publish `ScoreChangedEvent` cho UI vẽ lại — **không tự vẽ số điểm** (giữ đúng ranh giới UI/Logic).
- **Camera**: `CameraSystem` follow Player theo trục X với "dead zone" + smoothing (lerp), không bao giờ follow trục Y (chuẩn runner: camera chỉ cuộn ngang).
- **Animation**: State machine nhỏ (Idle/Run/Jump/Slide/Dead) độc lập khỏi input thật — nhận `AnimationState` component do `JumpSystem`/`SlideSystem` set, tự đổi frame theo `AnimationClip` load từ `AssetManager`.
- **Particle**: `ParticleSystem` dùng Object Pool, spawn theo Event (`CoinCollectedEvent` → sparkle, `PlayerDeadEvent` → burst) hoặc theo trigger liên tục (bụi khi chạy).
- **Audio**: `AudioSystem` là listener thuần tuý của Event Bus, không system nào khác được gọi trực tiếp `PlaySound()` — tất cả đi qua Event, giữ đúng "Observer" đã thiết kế ở mục 8.
- **Save**: xem mục 12.
- **Pause**: `GameStateMachine` push `PausedState` lên trên `PlayingState` (Stack), `FixedUpdate` của `PlayingState` không chạy khi bị che (nhưng entity vẫn giữ nguyên trong Registry) — publish `GamePausedEvent`/`GameResumedEvent` để UI và Audio phản ứng (VD giảm volume nhạc nền).
- **Menu**: `MenuState` là 1 UI Composite tree độc lập, publish `Event` khi bấm Play/Settings/Quit, không truy cập trực tiếp Gameplay System nào.
- **Death**: `CollisionSystem` phát hiện va chạm gây chết → publish `PlayerDeadEvent` → `GameStateMachine` chuyển `GameOverState` (xem luồng đầy đủ ở mục 5.4).
- **Restart**: `GameOverState::OnRestart()` gọi `SceneManager.ReloadScene()` — reset toàn bộ Registry sạch sẽ (tránh state rò rỉ giữa các lượt chơi), không tái sử dụng entity cũ.

**Tính độc lập giữa các hệ thống được đảm bảo bằng 2 quy tắc cứng:**
1. System A không bao giờ gọi hàm của System B trực tiếp — chỉ qua Component chung (đọc/ghi) hoặc Event Bus.
2. Thứ tự thực thi System trong 1 frame được khai báo tường minh ở 1 nơi duy nhất (`SystemScheduler` trong `SceneManager`), không suy luận ngầm từ thứ tự include.

---

## 11. EVENT SYSTEM (Event Bus)

### 11.1 Thiết kế

`EventBus` là 1 lớp mỏng, type-safe, dựa trên `std::type_index` + `std::function`, hỗ trợ **cả đồng bộ lẫn hàng đợi (queued)**:

- **Immediate dispatch**: dùng cho sự kiện cần phản ứng ngay trong cùng frame (hiếm khi cần).
- **Queued dispatch** (mặc định): publish trong lúc `FixedUpdate` chỉ đẩy vào queue, xử lý thật ở bước `EventBus.Dispatch()` cuối frame (mục 5.1) — tránh tình trạng 1 listener sửa Registry ngay giữa lúc System khác đang duyệt entity (undefined iterator invalidation).

### 11.2 Danh mục sự kiện cốt lõi

| Event | Publisher | Subscriber điển hình |
|---|---|---|
| `PlayerJumpEvent` | JumpSystem | AudioSystem, ParticleSystem, AnimationSystem |
| `PlayerDeadEvent` | CollisionSystem | GameStateMachine, SaveSystem, AchievementSystem, AudioSystem |
| `CoinCollectedEvent` | CollisionSystem | ScoreSystem, ParticleSystem, AudioSystem, AchievementSystem |
| `ObstacleSpawnedEvent` | ObstacleSpawnSystem | Debug overlay, (tương lai: AI/Boss trigger) |
| `ScoreChangedEvent` | ScoreSystem | UI HUD |
| `GamePausedEvent` / `GameResumedEvent` | GameStateMachine | UI, AudioSystem |
| `LevelCompletedEvent` | (dùng khi có chế độ Level thay vì Endless) | SceneManager, SaveSystem, UI |
| `AssetLoadedEvent` | ResourceManager | Debug loading screen, UI progress bar |

### 11.3 Giao tiếp module qua Event Bus — nguyên tắc

- Mọi Event là **struct POD bất biến** (định nghĩa ở `core/events`, không phụ thuộc engine/game cụ thể) → cho phép cả `Replay System` ghi log Event Bus lại thành file để phát lại chính xác (đây là lý do Event Bus nằm ở Layer 1/Core chứ không phải Layer 3).
- Publisher không bao giờ biết ai đang lắng nghe mình (đúng nghĩa Observer) — thêm `AchievementSystem` lắng nghe `CoinCollectedEvent` sau này không cần sửa `CollisionSystem`.

---

## 12. RESOURCE MANAGEMENT

### 12.1 Kiến trúc AssetManager

```
IAssetLoader<T>  (interface, VD: TextureLoader, AudioLoader, FontLoader)
        ▲
        │ implements
   TextureLoader / AudioLoader / FontLoader / AnimationLoader / ShaderLoader
        │
        ▼
   ResourceManager<T>  — template quản lý cache theo loại resource
        │  - std::unordered_map<AssetID, std::weak_ptr<T>>  (cache, không giữ chết resource không dùng)
        │  - Load(path) -> std::shared_ptr<T>
        │  - Preload(manifestFile)
        │  - Unload(AssetID) / UnloadUnused()
        ▼
   AssetManager  — Facade tổng hợp nhiều ResourceManager<T> (Texture/Audio/Font/Config...)
```

### 12.2 Cơ chế

- **Cache**: dùng `weak_ptr` làm key tra cứu — khi không còn `shared_ptr` nào giữ resource (VD scene đã unload), resource tự động đủ điều kiện unload mà không cần đếm tay.
- **Lazy loading**: resource chỉ thật sự đọc từ đĩa khi `Load()` được gọi lần đầu; các entity chỉ giữ `AssetHandle` (ID nhẹ) cho tới khi cần render mới resolve ra `shared_ptr` thật.
- **Preload**: `SceneManager` đọc `manifest.json` của scene sắp vào, gọi `AssetManager.Preload()` trong màn hình loading, tránh giật khi chơi (stutter khi lazy-load giữa gameplay).
- **Unload**: `SceneManager.UnloadScene()` gọi `AssetManager.UnloadUnused()` sau khi rời scene, dọn asset không còn ai tham chiếu.

### 12.3 Vì sao AssetManager không dùng raw pointer / index thủ công

`shared_ptr`/`weak_ptr` cho phép nhiều Entity (VD 50 obstacle cùng loại) share chung 1 Texture mà không lo double-free hay dangling pointer khi 1 obstacle bị destroy trước — đúng tinh thần Flyweight (mục 8) kết hợp RAII (mục 13).

---

## 13. MEMORY MANAGEMENT

| Kỹ thuật | Dùng khi nào trong dự án |
|---|---|
| **RAII** | Mọi resource hệ thống (SDL_Window*, SDL_Renderer*, file handle) được bọc trong wrapper class với destructor tự giải phóng — không bao giờ `SDL_DestroyWindow` rải rác thủ công. |
| **unique_ptr** | Sở hữu duy nhất, không chia sẻ: `IRenderer` do `Application` sở hữu, `IGameState` trong `GameStateMachine` stack, `System` instance trong `SystemScheduler`. |
| **shared_ptr** | Resource được nhiều nơi cùng tham chiếu và vòng đời không xác định trước rõ ràng: Texture/Audio/Font trong `AssetManager` (mục 12). |
| **weak_ptr** | Cache trong `ResourceManager` (không giữ sống resource chỉ vì nằm trong cache); tham chiếu ngược (VD `Particle` trỏ về `Emitter` cha) để tránh reference cycle với shared_ptr. |
| **Stack Allocation** | Component trong ECS (EnTT lưu trong pool liên tục, không heap-allocate từng entity riêng lẻ) — đây là lý do chính chọn ECS thay vì `vector<unique_ptr<GameObject>>`. |
| **Move Semantics** | Mọi struct Event, `AssetHandle`, kết quả trả về từ Factory đều hỗ trợ move constructor/assignment để tránh copy không cần thiết khi truyền qua Event Bus/Container. |
| **Rule of Five** | Áp dụng cho các class quản lý resource thô thủ công (VD wrapper `GLBuffer` nếu dùng OpenGL backend) — khi có custom destructor, phải định nghĩa tường minh cả copy/move ctor/assignment hoặc `= delete` chúng. |
| **Rule of Zero** | Áp dụng cho đa số class gameplay/domain — không có resource thô nào tự quản lý bằng tay, chỉ chứa `unique_ptr`/`shared_ptr`/STL container → không cần viết destructor/copy/move thủ công, để compiler tự sinh. |

**Nguyên tắc chọn**: mặc định luôn hướng tới **Rule of Zero**; chỉ khi thật sự bọc 1 resource hệ thống thô (con trỏ C API như SDL/OpenGL handle) mới rơi vào **Rule of Five**, và những class đó phải là các wrapper rất mỏng, cô lập ở đáy Layer 3 (Engine), không lan ra Gameplay code.

---

## 14. PERFORMANCE STRATEGY

- **Object Pool** cho Obstacle, Coin, Particle, Event — không `new/delete` trong hot loop (mục 8).
- **ECS cache-friendly**: EnTT lưu component theo sparse-set liên tục trong bộ nhớ, iterate `view<Transform, Velocity>()` tận dụng cache locality tốt hơn nhiều so với `vector<unique_ptr<GameObject>>` truy cập rải rác qua con trỏ.
- **Batching render**: gom sprite cùng texture thành 1 draw call (SDL3 Renderer hỗ trợ batching nội bộ khá tốt; nếu dùng GL backend, tự triển khai sprite batching qua 1 VBO động).
- **Giảm virtual call trong hot path**: `PhysicsSystem`/`CollisionSystem` thao tác trực tiếp trên component (struct thường, không virtual) — virtual chỉ dùng ở biên interface (VD `IRenderer`), không dùng cho từng obstacle riêng lẻ (đây cũng là lý do chọn ECS component thay vì polymorphism cho gameplay object).
- **Giảm copy**: truyền `const&`/`std::span` cho dữ liệu lớn (VD danh sách entity va chạm trong 1 frame), Event truyền theo giá trị nhưng struct nhỏ + move-friendly.
- **Culling**: `RenderSystem` chỉ vẽ entity nằm trong viewport (kết hợp Camera bounds), quan trọng khi TileMap dài.
- **Broad-phase collision**: dùng lưới không gian (spatial hashing) đơn giản theo trục X (vì runner chỉ cuộn 1 chiều) để tránh kiểm tra AABB kiểu O(n²) giữa mọi cặp entity khi obstacle nhiều.
- **Benchmark liên tục**: mọi thay đổi vào `CollisionSystem`/`ObjectPool` phải có benchmark Google Benchmark trước/sau để tránh tối ưu "cảm tính".

---

## 15. TESTING STRATEGY

| Loại test | Phạm vi | Công cụ | Ví dụ cụ thể |
|---|---|---|---|
| **Unit Test** | Logic thuần, không cần SDL3/GPU: `core/math`, `JumpSystem` (input → output velocity), `IDifficultyStrategy`, `ScoreSystem` | Catch2 + CTest | "Khi Player đang grounded và nhận JumpCommand, Velocity.y phải bằng -JumpForce cấu hình" |
| **Integration Test** | Nhiều System chạy chung qua 1 `Scene`/Registry thật, nhưng dùng `MockRenderer`/`MockAudioDevice` (không mở cửa sổ thật) | Catch2 + DI (mục 9, DIP) | "Sau N FixedUpdate với input Jump tại t=0, Player phải va chạm Obstacle đặt sẵn ở vị trí X và publish đúng `PlayerDeadEvent`" |
| **Gameplay Test** | Kịch bản chơi mô phỏng dài hơn (VD spawn 100 obstacle theo DifficultyStrategy, kiểm tra không có obstacle nào chồng lấn không thể né được) | Catch2 (custom fixtures), có thể kết hợp scripted input từ Replay format | Balance test: "Ở độ khó max, khoảng cách trung bình giữa 2 obstacle liên tiếp không nhỏ hơn khoảng cách né tối thiểu tính từ tốc độ Player" |
| **Regression Test** | So sánh output/replay giữa version cũ và mới cho cùng 1 seed | Replay System (mục Roadmap mở rộng) + Catch2 | "Chạy lại đúng 1 replay đã ghi trước đây, kết quả `finalScore` phải khớp — nếu lệch, có regression trong physics/collision" |

**Nguyên tắc:** vì Core/Gameplay layer không phụ thuộc SDL3 trực tiếp (DIP), CI có thể chạy toàn bộ Unit + Integration test **headless**, không cần GPU/display — quan trọng để chạy nhanh trên GitHub Actions/CI thông thường.

---

## 16. CODING STANDARDS (tóm tắt quy tắc)

Tham khảo **C++ Core Guidelines** làm nền, **Google C++ Style Guide** cho quy ước đặt tên/format, **LLVM Style** cho định dạng brace/indent — chốt lại bộ quy tắc riêng:

- **Naming**: `PascalCase` cho class/struct/enum, `camelCase` cho hàm/biến local, `m_` prefix cho member private (VD `m_velocity`), `UPPER_SNAKE_CASE` cho hằng số/`constexpr` toàn cục, `enum class` bắt buộc (không dùng `enum` trần).
- **Namespace**: mỗi Layer/Module có namespace riêng — `engine::renderer`, `engine::audio`, `game::player` — tránh `using namespace` trong header.
- **Header**: mọi header dùng `#pragma once`; include theo thứ tự chuẩn (C++ std → third-party → project), forward-declare khi có thể để giảm thời gian build.
- **Include**: không include chéo giữa Layer trái quy tắc mục 2 (kiểm tra bằng script `scripts/check_layer_deps.py` chạy trong CI).
- **Exception**: dùng exception cho lỗi khởi tạo nghiêm trọng (VD không load được asset bắt buộc), **không** dùng exception cho luồng điều khiển gameplay bình thường (VD "va chạm" không phải exception) — ưu tiên `std::optional`/`std::expected`-style return cho các trường hợp "có thể thất bại nhưng bình thường".
- **Const correctness**: tham số đọc-only truyền `const&`, method không sửa state đánh dấu `const`, ưu tiên `constexpr` cho hằng số biết trước lúc compile.
- **Formatting**: `.clang-format` dựa trên LLVM style, chạy tự động qua `scripts/format.sh` + pre-commit hook, CI fail nếu chưa format.

---

## 17. ROADMAP PHÁT TRIỂN

| Phase | Mục tiêu | Module liên quan | Kiến thức cần | Kết quả | Tiêu chí hoàn thành |
|---|---|---|---|---|---|
| **1. Project Setup** | Dựng khung CMake đa nền tảng, CI, coding standard, layer rỗng | cmake/, core/ (rỗng), tests/ (rỗng) | CMake modern, FetchContent, CI | Build "Hello Engine" trên 3 compiler | `cmake --build` chạy sạch trên GCC/Clang/MSVC, CI xanh |
| **2. Window & Application loop** | Mở cửa sổ, main loop fixed-timestep, xử lý thoát | engine/window, engine/application | SDL3 cơ bản, game loop pattern | Cửa sổ đen mở/đóng ổn định 60Hz | Đo được deltaTime ổn định, không leak SDL resource (kiểm bằng ASan) |
| **3. Rendering nền tảng** | Vẽ sprite tĩnh, camera cơ bản | engine/renderer, engine/camera | SDL3 Renderer/GL cơ bản, batching | Vẽ 1 sprite Player tĩnh lên màn hình | FPS ổn định 60 với vài trăm sprite test |
| **4. Input** | Bắt phím, Command pattern, rebind cơ bản | engine/input, core/events | SDL3 Event, Command pattern | Nhấn phím → publish InputCommand qua EventBus | Unit test InputMapper pass |
| **5. ECS Integration** | Tích hợp EnTT, định nghĩa Component core | core/ecs, engine/scene | EnTT API, data-oriented design | Spawn/destroy entity với Transform/Sprite | Integration test: entity render đúng vị trí |
| **6. Physics & Collision cơ bản** | Gravity, AABB collision | engine/physics, engine/collision | Fixed timestep integration, AABB | Player rơi tự do và dừng khi chạm nền test | Unit test physics pass, không "xuyên sàn" ở FPS thấp giả lập |
| **7. Player Movement (Jump/Slide)** | JumpSystem, SlideSystem, AnimationState cơ bản | game/player, engine/animation | State machine, coyote time | Player nhảy/trượt mượt, đổi animation đúng | Playtest cảm giác nhảy "đã tay", unit test JumpSystem |
| **8. Obstacle & Coin + Object Pool** | Spawn/pool obstacle, coin, va chạm gây chết/nhặt điểm | game/obstacles, game/coins, engine/collision | Object Pool pattern, Factory | Chạy né vật cản, nhặt coin cơ bản | Benchmark: spawn 1000 lần không leak, FPS ổn định |
| **9. Score & Difficulty Scaling** | ScoreSystem, IDifficultyStrategy | game/scoring, game/difficulty | Strategy pattern | Điểm tăng theo thời gian/khoảng cách, độ khó tăng dần | Gameplay test balance cơ bản pass |
| **10. Parallax Background & Particle** | Nhiều layer background, particle khi nhảy/chết/nhặt coin | engine/particles, resource | Parallax kỹ thuật, Object Pool | Background nhiều lớp cuộn khác tốc độ, particle mượt | Không giật frame khi particle burst lớn |
| **11. Audio** | SFX/nhạc nền theo Event | engine/audio | SDL3 Audio, Event-driven audio | Âm thanh nhảy/chết/coin/nhạc nền hoạt động | Audio không lệch nhịp, không leak channel |
| **12. UI, Menu, Pause, GameOver** | Toàn bộ GameStateMachine, UI Composite | game/states, engine/ui, engine/fonts | State pattern, Composite pattern | Luồng Menu→Play→Pause→GameOver→Restart hoàn chỉnh | Playtest full loop không crash, UI phản hồi đúng Event |
| **13. Save System** | High score, settings, cấu hình người chơi lưu/đọc | engine/save | cereal, JSON/binary serialization | Lưu/đọc high score qua các lần chạy | Integration test: ghi rồi đọc lại khớp dữ liệu |
| **14. Polish & Testing đầy đủ** | Hoàn thiện coverage test, fix bug, tối ưu | tests/, tools | Testing strategy mục 15 | Coverage đạt mục tiêu, benchmark ổn định | CI đầy đủ Unit+Integration+Benchmark xanh |
| **15. Packaging & Release MVP** | CPack đóng gói cho Win/Linux/Mac | cmake/Packaging.cmake | CPack, cross-platform install | Bản build cài đặt được trên cả 3 hệ điều hành | Cài đặt & chạy sạch trên máy sạch (VM test) |

**Sau MVP (Phase 16+, các nhánh mở rộng độc lập, không bắt buộc tuần tự):**
Achievement System → Replay System (dựa trên Command Pattern đã có từ Phase 4) → Multi-map/TileMap nâng cao → Multi-character (Prototype pattern) → Boss/AI module → Leaderboard online (Networking module) → Editor tool → Mod support (data-driven content qua JSON/Lua binding cân nhắc sau).

---

## 18. CÁC TÍNH NĂNG MỞ RỘNG TRONG TƯƠNG LAI (và kiến trúc đã chuẩn bị sẵn ra sao)

| Tính năng tương lai | Vì sao kiến trúc hiện tại đã sẵn sàng |
|---|---|
| **Nhiều map** | `TileMap` module tách khỏi Gameplay logic; `SceneBuilder` (Builder pattern) dựng scene từ file JSON — thêm map mới = thêm file JSON + asset, không sửa code. |
| **Nhiều nhân vật** | Player là Entity + Component data, không phải class cứng; `Prototype` pattern cho phép định nghĩa nhân vật mới qua data (stat, sprite sheet) mà không viết class mới. |
| **Nhiều chế độ chơi** | `IGameState`/`GameStateMachine` đã trừu tượng hoá luồng chơi; thêm `TimeAttackState`/`SurvivalState` chỉ là state mới, dùng lại toàn bộ System nền. |
| **Boss** | `AI` module đã có chỗ đứng riêng trong Dependency Graph (mục 4), dùng chung ECS + Event Bus với các entity khác — Boss chỉ là Entity với `AIBehaviorComponent` mới + `BossPhaseState` (State pattern). |
| **Online leaderboard** | `Networking` module là interface stub từ đầu; `Leaderboard` module đã tách khỏi `SaveSystem` cục bộ, chỉ cần cắm implementation HTTP client thật vào `INetworkClient`. |
| **Achievement** | Đã là module riêng lắng nghe Event Bus từ Phase MVP — thêm điều kiện mới chỉ là thêm rule data, không sửa Gameplay System. |
| **Mod support** | Vì cấu hình balance (`game/config`), obstacle definition, level data đều đã là JSON data-driven (không hard-code), bước tiếp theo tự nhiên là mở thư mục đó cho mod đọc/ghi — kiến trúc không cần đổi, chỉ cần thêm lớp validate/sandbox. |
| **Replay system** | `Command` pattern (Input) + Event Bus dạng POD serializable (cereal) từ đầu → Replay chỉ là "ghi lại chuỗi Command theo frame" và "phát lại qua đúng InputSystem" — không cần kiến trúc mới. |
| **Editor** | Editor dùng lại `engine/` (Renderer, Scene, ECS) mà không phụ thuộc `game/` — vì Dependency Graph mục 4 đã tách rạch ròi, Editor có thể build như 1 executable khác cùng dùng chung thư viện Engine. |

---

## 19. NHỮNG LỖI KIẾN TRÚC CẦN TRÁNH

1. **God Class "Game" ôm hết mọi thứ** — mọi state, mọi logic, mọi render đều nhét vào 1 class `Game::Update()` khổng lồ. Vi phạm SRP nghiêm trọng nhất, và là lỗi phổ biến nhất khi coder viết runner "for fun".
2. **Renderer leak vào Gameplay logic** — `JumpSystem` gọi thẳng `SDL_RenderCopy`. Phá vỡ ranh giới Layer 2/Layer 3, khiến gameplay không thể unit test headless.
3. **UI gọi thẳng Physics/Save** — Button "Pause" trong UI gọi trực tiếp `physicsSystem.Pause()` thay vì publish Event. Gây coupling chéo Layer không kiểm soát được, vi phạm đúng điều đề bài yêu cầu tránh.
4. **Lạm dụng kế thừa sâu cho Obstacle** (`SpikeObstacle : FlyingObstacle : Obstacle : Entity : GameObject`) — dẫn tới diamond problem, khó tái tổ hợp hành vi (VD "vừa bay vừa nổ"), nên dùng Component composition thay thế.
5. **Singleton tràn lan** (`ResourceManager::Instance()`, `AudioSystem::Instance()`, `EventBus::Instance()` ở mọi nơi) — biến toàn bộ codebase thành global state ẩn, không test được, không multi-instance được (VD chạy 2 Scene song song cho split-screen sau này là bất khả thi).
6. **new/delete rải rác trong hot loop** (spawn Obstacle/Particle mỗi frame bằng `new`) — gây phân mảnh heap, giật lag định kỳ, đặc biệt nghiêm trọng trên thiết bị yếu.
7. **Config/balance hard-code trong code** (`if (score > 1000) speed = 5.2f;` viết cứng trong `PlayerController`) — mỗi lần chỉnh balance phải build lại, designer không tự làm được, và không thể A/B test.
8. **Serialization không tách khỏi trạng thái runtime** — lưu game bằng cách `memcpy` thẳng cấu trúc Entity trong bộ nhớ ra file. Sẽ vỡ ngay khi đổi bất kỳ field nào ở version sau (không backward-compatible); phải dùng schema serialization rõ ràng (cereal) tách biệt DTO khỏi runtime struct.
9. **Trộn lẫn Fixed Update và Variable Update logic** — đặt physics integration vào `Update(deltaTime)` biến thiên theo FPS thay vì `FixedUpdate` cố định → hành vi game khác nhau giữa máy 30fps và 144fps, phá vỡ balance và gây bug khó tái hiện.
10. **Không có ranh giới rõ giữa Engine và Game** — code Engine import ngược lại thứ gì đó từ `game/` (VD `engine/renderer` biết về `PlayerSprite` cụ thể) khiến Engine không thể tái sử dụng cho game thứ 2, phá vỡ toàn bộ lý do tồn tại của Clean Architecture đã chọn.

---

## 20. GHI CHÚ TRIỂN KHAI

Đúng theo yêu cầu đề bài: tài liệu này là **thiết kế đầy đủ trước khi viết code**. Việc viết code sẽ **chỉ bắt đầu sau khi thiết kế được xác nhận**, và sẽ đi theo đúng trình tự **Phase 1 → Phase 15** ở mục 17, mỗi Phase kết thúc bằng 1 bản build chạy được + test tương ứng pass, trước khi sang Phase kế tiếp.

**Câu hỏi cần xác nhận trước khi bắt đầu Phase 1:**
- Renderer mặc định: chỉ dùng **SDL3 Renderer** (đơn giản, đủ cho 2D, khuyến nghị cho MVP), hay bật luôn **OpenGL backend** song song ngay từ đầu (phức tạp hơn, chỉ cần nếu biết chắc sẽ cần custom shader sớm)?
- Test framework: chốt **Catch2** hay **GoogleTest** làm chuẩn chính thức?
- Nền tảng ưu tiên build/test đầu tiên: Windows, Linux, hay cả hai song song từ Phase 1?
