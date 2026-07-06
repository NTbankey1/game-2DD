#include "ExplorationState.hpp"
#include "PausedState.hpp"
#include "GameOverState.hpp"
#include "engine/application/Application.hpp"
#include "engine/scene/GameStateMachine.hpp"
#include "engine/platform/sdl3/SDLRenderer.hpp"
#include "engine/renderer/TextRenderer.hpp"
#include "engine/renderer/RenderSystem.hpp"
#include "engine/renderer/RenderComponents.hpp"
#include "engine/physics/PhysicsSystem.hpp"
#include "engine/physics/PhysicsComponents.hpp"
#include "engine/physics/CollisionSystem.hpp"
#include "engine/tilemap/Tilemap.hpp"
#include "game/player/PlayerMovement.hpp"
#include "game/player/PlayerAnimationBindings.hpp"
#include "game/item/ItemComponent.hpp"
#include "game/item/ItemDatabase.hpp"
#include "game/combat/CombatComponent.hpp"
#include "game/interaction/InteractionSystem.hpp"
#include "engine/save/SaveManager.hpp"
#include "game/world/PortalComponent.hpp"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>
#include <string>
#include <cstdlib>
#include <cmath>
#include <algorithm>

namespace game {

static constexpr float W = 1280.0f;
static constexpr float H = 720.0f;
static constexpr float FIXED_DT = 1.0f / 60.0f;

ExplorationState::ExplorationState(engine::application::Application& app)
    : m_app(app)
    , m_inventory(app.Events())
    , m_inventoryUI(app.Events(), app.Text())
    , m_dialogueSystem(app.Events())
    , m_dialogueUI(app.Events(), app.Text())
    , m_questSystem(app.Events())
    , m_questLogUI(app.Text())
    , m_minimapUI(app.Text())
    , m_achievements(app.Events()) {
    ItemDatabase::Instance();

    // Register quest
    QuestDef coinQuest;
    coinQuest.id = "coin_collector";
    coinQuest.name = "Coin Collector";
    coinQuest.description = "Collect coins";
    coinQuest.objectives = {{QuestObjectiveType::CollectItem, "", 5, 0}};
    m_questSystem.RegisterQuest(coinQuest);

    // Register test dialogue
    DialogueData greet;
    greet.id = "test_greeting";
    greet.nodes = {
        {"Wanderer", "Hello there! Welcome to this world."},
        {"Wanderer", "I used to explore these lands too, before the great quake."},
        {"Wanderer", "If you find any coins, hold onto them. They're rare now."},
        {"Wanderer", "Safe travels, friend!"},
    };
    m_dialogueSystem.RegisterDialogue(greet);

    engine::animation::AnimState idle;
    idle.frames = {{0, 0, 0.5f}};
    idle.loop = true;
    m_animCtrl.AddState("Idle", idle);

    engine::animation::AnimState run;
    run.frames = {{0, 0, 0.08f}, {0, 1, 0.08f}, {0, 2, 0.08f}, {0, 3, 0.08f}};
    run.loop = true;
    m_animCtrl.AddState("Run", run);

    engine::animation::AnimState jump;
    jump.frames = {{0, 2, 0.15f}};
    jump.loop = false;
    m_animCtrl.AddState("Jump", jump);

    engine::animation::AnimState fall;
    fall.frames = {{0, 2, 0.2f}};
    fall.loop = false;
    m_animCtrl.AddState("Fall", fall);

    engine::animation::AnimState wallSlide;
    wallSlide.frames = {{0, 1, 0.12f}, {0, 0, 0.12f}};
    wallSlide.loop = true;
    m_animCtrl.AddState("WallSlide", wallSlide);

    engine::animation::AnimState dash;
    dash.frames = {{0, 0, 0.05f}, {0, 1, 0.05f}, {0, 2, 0.05f}, {0, 3, 0.05f}};
    dash.loop = true;
    m_animCtrl.AddState("Dash", dash);
}

ExplorationState::~ExplorationState() = default;

void ExplorationState::OnEnter() {
    spdlog::info("ExplorationState::OnEnter");
    m_prevCounter = SDL_GetPerformanceCounter();
    m_counterFreq = SDL_GetPerformanceFrequency();
    m_accumulator = 0.0f;
    m_hasInitialized = false;
    m_playerHp = m_playerMaxHp;

    // Subscribe to damage events
    m_dmgHandle = m_app.Events().Subscribe<DamageDealtEvent>(
        [this](const DamageDealtEvent& e) {
            // Check if target is the player
            auto& reg = m_app.Registry();
            for (auto pe : reg.view<engine::physics::PlayerTag>()) {
                if (e.target == pe) {
                    m_playerHp -= e.amount;
                    // Apply knockback
                    if (auto* vel = reg.try_get<engine::physics::VelocityComponent>(pe)) {
                        vel->velocity.x += e.knockback.x;
                        vel->velocity.y += e.knockback.y;
                    }
                    if (m_playerHp <= 0) {
                        m_app.SetGameOver(true);
                    }
                }
            }
        });
    m_enemyDeathHandle = m_app.Events().Subscribe<EnemyDiedEvent>(
        [this](const EnemyDiedEvent& e) {
            if (auto* dp = m_app.Registry().try_get<engine::renderer::TransformComponent>(e.entity)) {
                m_app.EmitParticles(dp->position.x + 16, dp->position.y + 16, 15, 0x66, 0xCC, 0x66, 80.0f);
                m_app.EmitParticles(dp->position.x + 16, dp->position.y + 16, 8, 0xCC, 0xFF, 0x66, 50.0f);
            }
            m_app.Registry().destroy(e.entity);
        });
}

void ExplorationState::OnExit() {
    spdlog::info("ExplorationState::OnExit");
    m_app.Events().Unsubscribe(m_dmgHandle);
    m_app.Events().Unsubscribe(m_enemyDeathHandle);
}

void ExplorationState::FixedUpdate(float /* dt */) {}

void ExplorationState::Update(float /*dt*/) {}

void ExplorationState::Render() {
    // Guard: don't run if app state has been changed by GameOver transition
    if (m_app.GetAppState() != engine::application::AppState::Playing) return;

    auto& app = m_app;
    auto& registry = app.Registry();
    auto& eventBus = app.Events();

    std::uint64_t curr = SDL_GetPerformanceCounter();
    float deltaTime = static_cast<float>(curr - m_prevCounter) / static_cast<float>(m_counterFreq);
    m_prevCounter = curr;
    if (deltaTime > 0.25f) deltaTime = 0.25f;

    // Build PlayerInput
    PlayerInput input;
    input.hInput = 0.0f;
    if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_LEFT] || SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_A])
        input.hInput -= 1.0f;
    if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_RIGHT] || SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_D])
        input.hInput += 1.0f;
    input.jumpHeld = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_SPACE]
                  || SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_UP]
                  || SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_W];
    input.jumpPressed = input.jumpHeld;
    input.dashPressed = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_LSHIFT]
                     || SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_RSHIFT];

    if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_ESCAPE]) {
        app.States().PushScene(std::make_unique<PausedState>(app));
        return;
    }

    // Initialize world once
    if (!m_hasInitialized) {
        app.Registry().clear();
        app.SpawnPlayer();
        std::string tilemapPath = std::string(TILEMAP_DIR) + "/test_world.json";
        if (m_tilemap.LoadFromFile(tilemapPath))
            app.Camera().SetBounds(core::Rectf(0.0f, 0.0f, m_tilemap.GetWorldWidth(), m_tilemap.GetWorldHeight()));
        m_collision.SetTilemap(&m_tilemap);
        SpawnWorldItems();
        m_hasInitialized = true;
    }

    // Fixed timestep physics
    // Add hurtbox to player if not present
    for (auto pe : registry.view<engine::physics::PlayerTag>()) {
        if (!registry.all_of<HurtboxComponent>(pe))
            registry.emplace<HurtboxComponent>(pe);
    }

    m_accumulator += deltaTime;
    bool gameOver = false;
    {
        int currentScore = app.Score();
        while (m_accumulator >= FIXED_DT) {
            m_physics.FixedUpdate(registry, FIXED_DT);
            m_collision.FixedUpdate(registry, eventBus, gameOver, currentScore);
            m_accumulator -= FIXED_DT;
        }
        app.SetScore(currentScore);
    }
    app.SetGameOver(gameOver);
    if (gameOver) {
        if (app.Anim().wasAlive) {
            app.Anim().wasAlive = false;
            for (auto pe : registry.view<engine::physics::PlayerTag>()) {
                if (auto* dp = registry.try_get<engine::renderer::TransformComponent>(pe)) {
                    app.EmitParticles(dp->position.x + 20.0f, dp->position.y + 25.0f,
                                      25, 0xFF, 0x44, 0x44, 150.0f);
                    app.EmitParticles(dp->position.x + 20.0f, dp->position.y + 25.0f,
                                      15, 0xFF, 0xCC, 0x00, 100.0f);
                    app.EmitParticles(dp->position.x + 20.0f, dp->position.y + 25.0f,
                                      10, 0xFF, 0xFF, 0xFF, 60.0f);
                }
            }
            app.ShakeAmount() = 8.0f;
        }
        app.SetAppState(engine::application::AppState::GameOver);
        app.States().PushScene(std::make_unique<GameOverState>(app));
        return;
    }

    // ===== PLAYER MOVEMENT =====
    // Jump buffer
    if (input.jumpHeld) {
        for (auto e : registry.view<engine::physics::PlayerStateComponent>()) {
            auto& ps = registry.get<engine::physics::PlayerStateComponent>(e);
            if (ps.jumpBufferTimer == 0.0f) ps.jumpBufferTimer = 0.001f;
        }
    }

    // Jump logic with coyote time
    for (auto e : registry.view<engine::physics::PlayerStateComponent>()) {
        auto& ps = registry.get<engine::physics::PlayerStateComponent>(e);
        ps.jumpHeld = input.jumpHeld;
        bool canJump = (ps.isGrounded || ps.coyoteTimer < 0.07f)
                    && ps.jumpBufferTimer > 0.0f
                    && ps.jumpBufferTimer < 0.10f;
        if (canJump && registry.all_of<engine::physics::VelocityComponent>(e)) {
            auto& vel = registry.get<engine::physics::VelocityComponent>(e);
            vel.velocity.y = -500.0f;
            ps.isGrounded = false;
            ps.coyoteTimer = 99.0f;
            app.Anim().jumpSquashTimer = 0.15f;
            if (auto* tp = registry.try_get<engine::renderer::TransformComponent>(e))
                app.EmitParticles(tp->position.x + 20.0f, tp->position.y + 52.0f, 10, 0xAA, 0x88, 0x55, 80.0f);
        }
        if (ps.jumpBufferTimer > 0.10f) ps.jumpBufferTimer = 0.0f;
        if (ps.isGrounded && !app.Anim().wasGrounded) {
            if (auto* pos = registry.try_get<engine::renderer::TransformComponent>(e))
                app.EmitParticles(pos->position.x + 20.0f, pos->position.y + 50.0f, 8, 0x8B, 0x6B, 0x4A, 60.0f);
            app.Anim().jumpSquashTimer = -0.1f;
        }
        app.Anim().wasGrounded = ps.isGrounded;
    }

    // PlayerMovement (walk, dash, wall-slide state tracking)
    m_playerMovement.Update(registry, input, deltaTime);

    // ===== PLAYER ANIMATION =====
    for (auto e : registry.view<engine::physics::PlayerTag>()) {
        auto& pos = registry.get<engine::renderer::TransformComponent>(e);
        auto& sprite = registry.get<engine::renderer::SpriteComponent>(e);

        // Flip sprite based on facing direction
        if (input.hInput < -0.1f)
            pos.scale.x = -1.0f;
        else if (input.hInput > 0.1f)
            pos.scale.x = 1.0f;

        // Animation via controller + bindings
        bool isGrounded = false;
        if (auto* ps = registry.try_get<engine::physics::PlayerStateComponent>(e))
            isGrounded = ps->isGrounded;

        float frame = 0.0f;
        ::game::UpdatePlayerAnimation(m_animCtrl, m_playerMovement.GetState(), isGrounded, deltaTime, frame, input.hInput);
        sprite.sourceRect.position.x = frame * 40.0f;

        // Squash/stretch
        auto& anim = app.Anim();
        float sx = pos.scale.x < 0 ? -1.0f : 1.0f;
        if (anim.jumpSquashTimer > 0.0f) {
            float t = std::min(anim.jumpSquashTimer / 0.12f, 1.0f);
            pos.scale.x = sx * (1.0f + (1.15f - 1.0f) * (1.0f - t));
            pos.scale.y = 1.0f - (1.0f - 0.85f) * (1.0f - t);
            anim.jumpSquashTimer -= deltaTime;
            if (anim.jumpSquashTimer <= 0.0f) { pos.scale.x = sx; pos.scale.y = 1.0f; }
        } else if (anim.jumpSquashTimer < 0.0f) {
            float t = std::min(-anim.jumpSquashTimer / 0.08f, 1.0f);
            pos.scale.x = sx * (1.0f + (1.1f - 1.0f) * t);
            pos.scale.y = 1.0f - (1.0f - 0.9f) * t;
            anim.jumpSquashTimer += deltaTime;
            if (anim.jumpSquashTimer >= 0.0f) { pos.scale.x = sx; pos.scale.y = 1.0f; }
        } else {
            pos.scale.y = 1.0f;
        }
    }

    // Camera
    UpdateCamera(deltaTime);

    // Interaction & Dialogue
    static bool prevE = false;
    bool eNow = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_E];
    bool ePressed = eNow && !prevE;
    prevE = eNow;

    if (m_dialogueSystem.IsActive()) {
        if (ePressed) {
            if (!m_dialogueSystem.Advance()) {
                // Dialogue ended — start quest if NPC talked to
                if (!m_questSystem.IsCompleted("coin_collector")) {
                    auto* prog = m_questSystem.GetProgress("coin_collector");
                    if (!prog) m_questSystem.StartQuest("coin_collector");
                }
            }
        }
    } else {
        entt::entity interactTarget = entt::null;
        bool nearInteractable = m_interaction.CheckProximity(registry, interactTarget);
        if (ePressed && nearInteractable) {
            if (interactTarget == m_chestEntity && !m_chestOpened) {
                m_chestOpened = true;
                m_inventory.AddItem("key", 1);
                m_inventory.AddItem("coin", 10);
                app.EmitParticles(200.0f, 560.0f, 25, 0xFF, 0xAA, 0x00, 100.0f);
                if (registry.all_of<InteractableTag>(m_chestEntity))
                    registry.remove<InteractableTag>(m_chestEntity);
            } else if (interactTarget == m_npcEntity) {
                m_dialogueSystem.StartDialogue("test_greeting");
            }
        }
    }

    // Enemy AI + Combat (after physics, before particles)
    m_enemyFSM.Update(registry, deltaTime, 100.0f, 500.0f);
    m_bossFSM.Update(registry, deltaTime, 100.0f, 500.0f);
    m_combatSystem.FixedUpdate(registry, app.Events(), deltaTime);

    // Checkpoint activation
    m_checkpointSystem.CheckActivation(registry, 100.0f, 500.0f);

    // Portal teleport
    {
        core::Vec2f portalDest;
        if (m_portalSystem.CheckPortal(registry, portalDest, 100.0f, 500.0f)) {
            for (auto pe : registry.view<engine::physics::PlayerTag>()) {
                auto& pp = registry.get<engine::renderer::TransformComponent>(pe);
                pp.position = portalDest;
                if (auto* vel = registry.try_get<engine::physics::VelocityComponent>(pe))
                    vel->velocity = {};
                app.EmitParticles(portalDest.x, portalDest.y, 20, 0x88, 0x44, 0xFF, 100.0f);
                break;
            }
        }
    }

    // Save/Load (F5/F9)
    static bool prevF5 = false, prevF9 = false;
    bool f5 = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_F5];
    bool f9 = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_F9];
    if (f5 && !prevF5) {
        GameSaveData data;
        data.score = m_app.Score();
        data.highScore = m_app.HighScore();
        data.coins = m_app.Coins();
        data.playerHp = m_playerHp;
        for (auto pe : registry.view<engine::physics::PlayerTag>()) {
            auto& pp = registry.get<engine::renderer::TransformComponent>(pe);
            data.playerX = pp.position.x;
            data.playerY = pp.position.y;
        }
        engine::save::SaveManager::SaveToFile("save.json", data.ToJson());
    }
    if (f9 && !prevF9) {
        auto j = engine::save::SaveManager::LoadFromFile("save.json");
        if (!j.empty()) {
            auto data = GameSaveData::FromJson(j);
            m_app.SetScore(data.score);
            m_playerHp = data.playerHp;
            for (auto pe : registry.view<engine::physics::PlayerTag>()) {
                auto& pp = registry.get<engine::renderer::TransformComponent>(pe);
                pp.position.x = data.playerX;
                pp.position.y = data.playerY;
            }
        }
    }
    prevF5 = f5; prevF9 = f9;

    // Check player death
    if (m_playerHp <= 0) {
        // Respawn at checkpoint instead of game over
        auto respawn = m_checkpointSystem.GetRespawnPos(registry);
        m_playerHp = m_playerMaxHp;
        for (auto pe : registry.view<engine::physics::PlayerTag>()) {
            auto& pp = registry.get<engine::renderer::TransformComponent>(pe);
            pp.position = respawn;
            if (auto* vel = registry.try_get<engine::physics::VelocityComponent>(pe))
                vel->velocity = {};
        }
    }

    // Particles & popups
    app.UpdateParticles(deltaTime);
    app.UpdatePopups(deltaTime);

    // === RENDER ===
    app.BeginFrame();
    RenderBackground();
    app.RenderParticles();

    if (app.ShakeAmount() > 0.5f) {
        auto save = app.Camera().GetPosition();
        float sx = (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 2.0f * app.ShakeAmount();
        float sy = (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 2.0f * app.ShakeAmount();
        app.Camera().SetPosition(save + core::Vec2f(sx, sy));
        app.RenderSys().Render(registry, app.Camera());
        app.Camera().SetPosition(save);
        app.ShakeAmount() *= 0.85f;
        if (app.ShakeAmount() < 0.3f) app.ShakeAmount() = 0.0f;
    } else {
        app.RenderSys().Render(registry, app.Camera());
        app.ShakeAmount() = 0.0f;
    }

    app.RenderPopups();
    RenderHUD();

    // Minimap (update discovery + render)
    m_minimapUI.Discover(app.Camera().GetPosition(), 1280.0f, 720.0f,
                         m_tilemap.GetWorldWidth(), m_tilemap.GetWorldHeight());
    m_minimapUI.Render(W - 140.0f, 90.0f, 120.0f, 80.0f,
                       app.Camera().GetPosition() + core::Vec2f(640, 360),
                       m_tilemap.GetWorldWidth(), m_tilemap.GetWorldHeight());

    // Dialogue box (on top of everything)
    if (m_dialogueSystem.IsActive()) {
        m_dialogueUI.Render(app.Renderer().Handle(), m_dialogueSystem);
    }

    // Interaction prompt
    if (!m_dialogueSystem.IsActive()) {
        entt::entity tgt = entt::null;
        if (m_interaction.CheckProximity(registry, tgt)) {
            float pulse = 0.7f + 0.3f * std::sin(app.MenuTimer() * 4.0f);
            uint8_t a = static_cast<uint8_t>(pulse * 255.0f);
            const char* prompt = "[E] Interact";
            if (tgt == m_chestEntity && !m_chestOpened) prompt = "[E] Open Chest";
            else if (tgt == m_npcEntity) prompt = "[E] Talk";
            float pw2 = static_cast<float>(app.Text().TextWidth(prompt, 18));
            app.Text().RenderString(prompt, 640 - pw2 / 2.0f, 350, 0xFF, static_cast<uint8_t>(0xCC * pulse), a, 18);
        }
    }

    app.EndFrame();
}

void ExplorationState::UpdateCamera(float /*dt*/) {
    for (auto e : m_app.Registry().view<engine::physics::PlayerTag>()) {
        auto& pos = m_app.Registry().get<engine::renderer::TransformComponent>(e);
        m_app.Camera().FollowTarget(e, pos.position, 0.08f);
        break;
    }
}

void ExplorationState::RenderBackground() {
    m_app.DrawGradientBackground(0x30, 0x50, 0x80, 0x10, 0x20, 0x50);
    m_app.DrawStars(0.3f);
    m_app.DrawClouds();
    m_app.DrawParallaxMountains();
}

void ExplorationState::SpawnWorldItems() {
    auto& registry = m_app.Registry();

    // Portal at x=300 → teleports to x=850 (past platforming gap)
    auto portal = registry.create();
    registry.emplace<engine::renderer::TransformComponent>(portal,
        core::Vec2f(300.0f, 544.0f), 0.0f, core::Vec2f(1.0f, 1.0f));
    registry.emplace<PortalComponent>(portal, core::Vec2f(850.0f, 540.0f));

    // Boss at x=2900 (boss arena, end of world)
    auto boss = registry.create();
    registry.emplace<engine::renderer::TransformComponent>(boss,
        core::Vec2f(2900.0f, 544.0f), 0.0f, core::Vec2f(1.0f, 1.0f));
    registry.emplace<engine::physics::VelocityComponent>(boss);
    registry.emplace<engine::physics::GravityComponent>(boss, 980.0f);
    registry.emplace<engine::physics::AABBComponent>(boss, core::Rectf(0, 0, 48, 48));
    registry.emplace<EnemyComponent>(boss, EnemyType::Default, 10, 10, 2, 60.0f, 40.0f, 300.0f);
    registry.emplace<HitboxComponent>(boss);
    registry.emplace<HurtboxComponent>(boss);
    registry.emplace<engine::renderer::SpriteComponent>(boss,
        m_app.ObstacleTexturePtr(), core::Rectf(0, 0, 32, 48), 3);

    // Checkpoint at x=1500 (pre-enemy-area)
    auto cp = registry.create();
    registry.emplace<engine::renderer::TransformComponent>(cp,
        core::Vec2f(1500.0f, 544.0f), 0.0f, core::Vec2f(1.0f, 1.0f));
    registry.emplace<CheckpointComponent>(cp, core::Vec2f(1500.0f, 500.0f));

    // Slime enemy at x=1600 (enemy area between rocks)
    auto slime = registry.create();
    registry.emplace<engine::renderer::TransformComponent>(slime,
        core::Vec2f(1600.0f, 544.0f), 0.0f, core::Vec2f(1.0f, 1.0f));
    registry.emplace<engine::physics::VelocityComponent>(slime);
    registry.emplace<engine::physics::GravityComponent>(slime, 980.0f);
    registry.emplace<engine::physics::AABBComponent>(slime, core::Rectf(4, 0, 24, 32));
    registry.emplace<EnemyComponent>(slime, EnemyType::Slime, 3, 3, 1, 60.0f, 30.0f, 200.0f);
    registry.emplace<HitboxComponent>(slime);
    registry.emplace<HurtboxComponent>(slime);
    registry.emplace<engine::renderer::SpriteComponent>(slime,
        m_app.CoinTexturePtr(), core::Rectf(0, 0, 32, 32), 3);

    // Chest at x=650 (safe alcove)
    m_chestEntity = registry.create();
    registry.emplace<engine::renderer::TransformComponent>(m_chestEntity,
        core::Vec2f(650.0f, 544.0f), 0.0f, core::Vec2f(1.0f, 1.0f));
    registry.emplace<engine::renderer::SpriteComponent>(m_chestEntity,
        m_app.CoinTexturePtr(), core::Rectf(0, 0, 32, 32), 3);
    registry.emplace<engine::physics::AABBComponent>(m_chestEntity, core::Rectf(0, 0, 32, 32));
    registry.emplace<InteractableTag>(m_chestEntity);
    registry.emplace<ItemComponent>(m_chestEntity, "key", 1);

    // NPC Wanderer at x=450
    m_npcEntity = registry.create();
    registry.emplace<engine::renderer::TransformComponent>(m_npcEntity,
        core::Vec2f(450.0f, 544.0f), 0.0f, core::Vec2f(1.0f, 1.0f));
    registry.emplace<engine::renderer::SpriteComponent>(m_npcEntity,
        m_app.PlayerSheetTexturePtr(), core::Rectf(0, 0, 40, 54), 3);
    registry.emplace<InteractableTag>(m_npcEntity);
    registry.emplace<NPCComponent>(m_npcEntity, "Wanderer", "test_greeting");

    // Coins scattered along the path
    for (int i = 0; i < 8; i++) {
        auto coin = registry.create();
        float cx = 200.0f + static_cast<float>(i) * 150.0f;
        registry.emplace<engine::renderer::TransformComponent>(coin,
            core::Vec2f(cx, 570.0f), 0.0f, core::Vec2f(1.0f, 1.0f));
        registry.emplace<engine::physics::AABBComponent>(coin, core::Rectf(0, 0, 16, 16));
        registry.emplace<ItemComponent>(coin, "coin", 1);
    }
    // Coin pickups use AABB overlap
    // They'll be handled by proximity + E like the chest
    // Simpler: make coins auto-pickup via AABB check
}

void ExplorationState::RenderHUD() {
    // Dark HUD bar at top
    SDL_SetRenderDrawBlendMode(m_app.Renderer().Handle(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(m_app.Renderer().Handle(), 0x08, 0x0A, 0x18, 200);
    SDL_FRect hudBg{0, 0, W, 36};
    SDL_RenderFillRect(m_app.Renderer().Handle(), &hudBg);
    // Accent line
    SDL_SetRenderDrawColor(m_app.Renderer().Handle(), 0x44, 0x66, 0xCC, 180);
    SDL_FRect accent{0, 36, W, 2};
    SDL_RenderFillRect(m_app.Renderer().Handle(), &accent);
    SDL_SetRenderDrawBlendMode(m_app.Renderer().Handle(), SDL_BLENDMODE_NONE);

    // HP hearts
    std::string hpStr;
    for (int i = 0; i < m_playerMaxHp; i++)
        hpStr += (i < m_playerHp) ? "♥ " : "♡ ";
    m_app.Text().RenderString(hpStr.c_str(), 15, 7, 0xFF, 0x66, 0x66, 18);

    // Controls
    m_app.Text().RenderString("WASD | E interact | SPACE jump | SHIFT dash", 120, 7, 0x66, 0x88, 0xAA, 14);
    m_app.Text().RenderString("F5 save | F9 load", 720, 7, 0x55, 0x66, 0x88, 14);

    // Coins
    m_app.Text().RenderString(("Coins: " + std::to_string(m_app.Coins())).c_str(), W - 130, 7, 0xFF, 0xDD, 0x00, 16);

    // Inventory panel (right)
    float invX = W - 200.0f, invY = 50.0f;
    SDL_SetRenderDrawBlendMode(m_app.Renderer().Handle(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(m_app.Renderer().Handle(), 0x08, 0x0A, 0x18, 180);
    SDL_FRect invBg{invX, invY, 180, 100};
    SDL_RenderFillRect(m_app.Renderer().Handle(), &invBg);
    SDL_SetRenderDrawColor(m_app.Renderer().Handle(), 0x44, 0x66, 0xCC, 100);
    SDL_FRect invAcc{invX, invY, 180, 1};
    SDL_RenderFillRect(m_app.Renderer().Handle(), &invAcc);
    SDL_SetRenderDrawBlendMode(m_app.Renderer().Handle(), SDL_BLENDMODE_NONE);
    m_inventoryUI.Render(invX + 10, invY + 10);

    // Quest log (left)
    auto activeQuests = m_questSystem.GetActiveQuests();
    if (!activeQuests.empty()) {
        float qx = 10, qy = 50;
        SDL_SetRenderDrawBlendMode(m_app.Renderer().Handle(), SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(m_app.Renderer().Handle(), 0x08, 0x0A, 0x18, 180);
        SDL_FRect qBg{qx, qy, 220, 100};
        SDL_RenderFillRect(m_app.Renderer().Handle(), &qBg);
        SDL_SetRenderDrawColor(m_app.Renderer().Handle(), 0x44, 0xCC, 0x66, 100);
        SDL_FRect qAcc{qx, qy, 220, 1};
        SDL_RenderFillRect(m_app.Renderer().Handle(), &qAcc);
        SDL_SetRenderDrawBlendMode(m_app.Renderer().Handle(), SDL_BLENDMODE_NONE);
        m_questLogUI.Render(qx + 10, qy + 10, activeQuests);
    }
}

} // namespace game
