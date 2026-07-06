#pragma once

#include "engine/scene/IScene.hpp"
#include "engine/physics/PhysicsSystem.hpp"
#include "engine/physics/CollisionSystem.hpp"
#include "engine/tilemap/Tilemap.hpp"
#include "engine/animation/AnimationController.hpp"
#include "game/player/PlayerMovement.hpp"
#include "game/inventory/InventorySystem.hpp"
#include "game/interaction/InteractionSystem.hpp"
#include "game/ui/InventoryUI.hpp"
#include "game/dialogue/DialogueSystem.hpp"
#include "game/dialogue/DialogueEvents.hpp"
#include "game/ui/DialogueUI.hpp"
#include "game/npc/NPCComponent.hpp"
#include "game/quest/QuestSystem.hpp"
#include "game/quest/QuestData.hpp"
#include "game/ui/QuestLogUI.hpp"
#include "game/combat/CombatSystem.hpp"
#include "game/ai/EnemyFSM.hpp"
#include "game/ai/BossFSM.hpp"
#include "game/enemy/EnemyComponent.hpp"
#include "game/checkpoint/CheckpointSystem.hpp"
#include "game/save/GameSaveData.hpp"
#include "game/world/PortalSystem.hpp"
#include "game/ui/MinimapUI.hpp"
#include "game/achievement/AchievementSystem.hpp"
#include <cstdint>

namespace engine::application { class Application; }

namespace game {

class ExplorationState : public engine::IScene {
public:
    explicit ExplorationState(engine::application::Application& app);
    ~ExplorationState() override;

    void OnEnter() override;
    void OnExit() override;
    void FixedUpdate(float dt) override;
    void Update(float dt) override;
    void Render() override;

private:
    engine::application::Application& m_app;
    engine::physics::PhysicsSystem m_physics;
    engine::physics::CollisionSystem m_collision;
    float m_accumulator = 0.0f;
    std::uint64_t m_prevCounter = 0;
    std::uint64_t m_counterFreq = 1;
    bool m_hasInitialized = false;

    PlayerMovement m_playerMovement;
    engine::animation::AnimationController m_animCtrl;
    InventorySystem m_inventory;
    InteractionSystem m_interaction;
    InventoryUI m_inventoryUI;
    DialogueSystem m_dialogueSystem;
    DialogueUI m_dialogueUI;
    QuestSystem m_questSystem;
    QuestLogUI m_questLogUI;
    CombatSystem m_combatSystem;
    EnemyFSM m_enemyFSM;
    BossFSM m_bossFSM;
    CheckpointSystem m_checkpointSystem;
    PortalSystem m_portalSystem;
    MinimapUI m_minimapUI;
    AchievementSystem m_achievements;
    entt::entity m_chestEntity = entt::null;
    entt::entity m_npcEntity = entt::null;
    bool m_chestOpened = false;
    int m_playerHp = 5;
    int m_playerMaxHp = 5;
    core::events::ListenerHandle m_dmgHandle;
    core::events::ListenerHandle m_enemyDeathHandle;

    void HandleJump(float dt, const PlayerInput& input);
    void HandleMovement(float dt);
    void UpdatePlayerAnimation(float dt, const PlayerInput& input);
    void UpdateCamera(float dt);
    void RenderBackground();
    void RenderHUD();
    void SpawnWorldItems();

    engine::tilemap::Tilemap m_tilemap;
};

} // namespace game
