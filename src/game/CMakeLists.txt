add_library(game_impl STATIC
    engine_impl/MenuState.cpp
    input/InputMapper.cpp
    item/ItemDatabase.cpp
    inventory/InventorySystem.cpp
    interaction/InteractionSystem.cpp
    dialogue/DialogueSystem.cpp
    quest/QuestSystem.cpp
    ui/InventoryUI.cpp
    ui/DialogueUI.cpp
    ui/QuestLogUI.cpp
    ui/MinimapUI.cpp
    world/PortalSystem.cpp
    achievement/AchievementSystem.cpp
    ai/EnemyFSM.cpp
    ai/BossFSM.cpp
    combat/CombatSystem.cpp
    checkpoint/CheckpointSystem.cpp
    save/GameSaveData.cpp
    player/PlayerMovement.cpp
    player/PlayerAnimationBindings.cpp
    states/ExplorationState.cpp
    states/PausedState.cpp
    states/GameOverState.cpp
)
target_include_directories(game_impl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(game_impl PRIVATE TILEMAP_DIR="${CMAKE_SOURCE_DIR}/assets/tilemaps")
target_link_libraries(game_impl PUBLIC engine_application engine_interface)
add_compiler_warnings(game_impl)
add_sanitizers(game_impl)
