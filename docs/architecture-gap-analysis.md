# Architecture Gap Analysis — Kết quả sau Refactor

> **Trạng thái: ĐÃ HOÀN THÀNH** (tất cả 4 phase)

## So sánh Before/After

| Khía cạnh | Trước | Sau |
|---|---|---|
| **Application::Run()** | 1171 dòng, switch-case 4 state | 230 dòng, menu + `m_stateMachine->Update/FixedUpdate/Render` |
| **Game state machine** | GameStateMachine không được dùng → switch-case | GameStateMachine thực sự drive Playing/Paused/GameOver |
| **Dead code** | 11 file chết + `game_stubs` target | Tất cả đã xoá |
| **File structure** | 58 files | **51 files** (xoá 11 chết, thêm 4 mới) |
| **Playing logic** | inline trong Application::Run() | 460 dòng trong PlayingState class riêng |
| **entity rendering** | MenuState::Render() rỗng | MenuState còn nhưng không ảnh hưởng (menu inline) |

## Những gì còn có thể làm (Phase tương lai)

- Object pool cho obstacle/coin entities
- EventBus publish/subscribe thật (hiện CollisionSystem publish nhưng chưa ai subscribe)
- Load texture từ PNG thay vì vẽ tay bằng SetPixel
- Audio system
