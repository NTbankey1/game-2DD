#include "Application.hpp"
#include "engine/platform/sdl3/SDLRenderer.hpp"
#include <SDL3/SDL.h>
#include <cstdint>
#include <string>

namespace engine::application {

void Application::EmitParticles(float x, float y, int count, uint8_t r, uint8_t g, uint8_t b, float spread) {
    for (int i = 0; i < count && m_particleCount < MAX_PARTICLES; i++) {
        auto& p = m_particles[m_particleCount];
        p.x = x;
        p.y = y;
        float angle = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 6.2832f;
        float spd = (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * spread + 20.0f;
        p.vx = std::cos(angle) * spd;
        p.vy = std::sin(angle) * spd - 30.0f;
        p.life = 0.2f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 0.3f;
        p.maxLife = p.life;
        p.r = r; p.g = g; p.b = b;
        p.a = 200;
        p.size = 2 + static_cast<uint8_t>((static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 3.0f);
        m_particleCount++;
    }
}

void Application::UpdateParticles(float dt) {
    for (int i = 0; i < m_particleCount; ) {
        auto& p = m_particles[i];
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.vy += 200.0f * dt;
        p.life -= dt;
        float t = std::max(p.life / p.maxLife, 0.0f);
        p.a = static_cast<uint8_t>(t * 200.0f);
        if (p.life <= 0.0f) {
            std::swap(p, m_particles[--m_particleCount]);
        } else {
            i++;
        }
    }
}

void Application::RenderParticles() {
    SDL_Renderer* r = m_renderer->Handle();
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < m_particleCount; i++) {
        const auto& p = m_particles[i];
        SDL_SetRenderDrawColor(r, p.r, p.g, p.b, p.a);
        float s = static_cast<float>(p.size);
        SDL_FRect rect{p.x - s / 2.0f, p.y - s / 2.0f, s, s};
        SDL_RenderFillRect(r, &rect);
    }
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}

void Application::SpawnScorePopup(int score, float x, float y) {
    for (int i = 0; i < MAX_POPUPS; i++) {
        if (!m_popups[i].active) {
            m_popups[i].x = x;
            m_popups[i].y = y;
            m_popups[i].score = score;
            m_popups[i].life = 0.8f;
            m_popups[i].active = true;
            return;
        }
    }
}

void Application::UpdatePopups(float dt) {
    for (int i = 0; i < MAX_POPUPS; i++) {
        if (!m_popups[i].active) continue;
        m_popups[i].y -= 40.0f * dt;
        m_popups[i].x -= 10.0f * dt;
        m_popups[i].life -= dt;
        if (m_popups[i].life <= 0.0f) m_popups[i].active = false;
    }
}

void Application::RenderPopups() {
    for (int i = 0; i < MAX_POPUPS; i++) {
        if (!m_popups[i].active) continue;
        float t = m_popups[i].life / 0.8f;
        uint8_t alpha = static_cast<uint8_t>(t * 255.0f);
        std::string txt = std::string("+") + std::to_string(m_popups[i].score);
        m_text->RenderString(txt.c_str(),
            static_cast<float>(static_cast<int>(m_popups[i].x)),
            m_popups[i].y,
            0xFF, static_cast<uint8_t>(t * 221.0f), alpha);
    }
}

} // namespace engine::application
