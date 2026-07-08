#include "Application.hpp"
#include <SDL3/SDL.h>
#include <cstdint>

namespace engine::application {

void SetPixel(SDL_Surface* surf, int x, int y, uint32_t color) {
    if (x < 0 || x >= surf->w || y < 0 || y >= surf->h) return;
    auto* pixels = static_cast<uint32_t*>(surf->pixels);
    pixels[y * (surf->pitch / 4) + x] = color;
}

static uint32_t RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA8888), nullptr, r, g, b, a);
}

static void DrawPlayerHead(SDL_Surface* surf, int ox, uint32_t skin, uint32_t hair, uint32_t hairL,
                            uint32_t whiteE, uint32_t pupil, uint32_t mouth, uint32_t blush) {
    for (int x = ox + 8; x < ox + 32; x++) {
        SetPixel(surf, x, 0, hair);
        SetPixel(surf, x, 1, hair);
    }
    SetPixel(surf, ox + 6, 1, hair); SetPixel(surf, ox + 7, 1, hair);
    SetPixel(surf, ox + 32, 1, hair); SetPixel(surf, ox + 33, 1, hair);
    for (int x = ox + 10; x < ox + 30; x++) {
        if (x < ox + 13 || x > ox + 27) SetPixel(surf, x, 2, hair);
    }
    SetPixel(surf, ox + 8, 2, hair); SetPixel(surf, ox + 9, 2, hair);
    SetPixel(surf, ox + 30, 2, hair); SetPixel(surf, ox + 31, 2, hair);
    SetPixel(surf, ox + 14, 0, hairL); SetPixel(surf, ox + 15, 0, hairL);
    SetPixel(surf, ox + 12, 1, hairL); SetPixel(surf, ox + 20, 1, hairL);

    for (int y = 2; y < 13; y++) {
        int halfW = (y == 2) ? 6 : (7 + (y - 2) / 2);
        if (y >= 11) halfW = 9;
        for (int x = ox + 20 - halfW; x < ox + 20 + halfW; x++)
            if (x >= ox && x < ox + 40) SetPixel(surf, x, y, skin);
    }
    for (int ex = ox + 14; ex <= ox + 17; ex++) { SetPixel(surf, ex, 5, whiteE); SetPixel(surf, ex, 6, whiteE); }
    for (int ex = ox + 22; ex <= ox + 25; ex++) { SetPixel(surf, ex, 5, whiteE); SetPixel(surf, ex, 6, whiteE); }
    SetPixel(surf, ox + 16, 6, pupil); SetPixel(surf, ox + 24, 6, pupil);
    SetPixel(surf, ox + 15, 5, pupil); SetPixel(surf, ox + 23, 5, pupil);
    SetPixel(surf, ox + 14, 5, pupil); SetPixel(surf, ox + 22, 5, pupil);
    for (int ex = ox + 13; ex <= ox + 16; ex++) SetPixel(surf, ex, 3, hair);
    for (int ex = ox + 23; ex <= ox + 26; ex++) SetPixel(surf, ex, 3, hair);
    for (int mx = ox + 18; mx <= ox + 21; mx++) SetPixel(surf, mx, 9, mouth);
    SetPixel(surf, ox + 11, 7, blush); SetPixel(surf, ox + 12, 7, blush);
    SetPixel(surf, ox + 27, 7, blush); SetPixel(surf, ox + 28, 7, blush);
}

static void DrawPlayerTorso(SDL_Surface* surf, int ox, uint32_t skin, uint32_t skinS,
                             uint32_t shirt, uint32_t shirtD) {
    for (int y = 13; y < 30; y++) {
        int left = (y < 16) ? 10 : 8;
        int right = (y < 16) ? 29 : 31;
        for (int x = ox + left; x <= ox + right; x++)
            SetPixel(surf, x, y, ((x + y) % 5 == 0) ? shirtD : shirt);
    }
    for (int y = 14; y < 26; y++) {
        for (int x = ox + 4; x < ox + 8; x++) SetPixel(surf, x, y, skin);
        for (int x = ox + 32; x < ox + 36; x++) SetPixel(surf, x, y, skin);
    }
    for (int y = 14; y < 26; y++) {
        SetPixel(surf, ox + 4, y, skinS); SetPixel(surf, ox + 35, y, skinS);
    }
    for (int x = ox + 10; x < ox + 30; x++) SetPixel(surf, x, 30, shirtD);
}

static void DrawPlayerLegs(SDL_Surface* surf, int ox, uint32_t pants, uint32_t shoes,
                            uint32_t skinS, int legOffsetL, int legOffsetR) {
    int lx = 11 + legOffsetL;
    int rx = 21 + legOffsetR;
    for (int y = 30; y < 42; y++) {
        for (int x = ox + lx; x < ox + lx + 8; x++) SetPixel(surf, x, y, pants);
    }
    for (int y = 30; y < 42; y++) {
        for (int x = ox + rx; x < ox + rx + 8; x++) SetPixel(surf, x, y, pants);
    }
    for (int y = 30; y < 36; y++)
        for (int x = ox + 18; x < ox + 22; x++) SetPixel(surf, x, y, pants);
    for (int y = 42; y < 52; y++) {
        for (int x = ox + lx - 1; x < ox + lx + 9; x++) SetPixel(surf, x, y, shoes);
        for (int x = ox + rx - 1; x < ox + rx + 9; x++) SetPixel(surf, x, y, shoes);
    }
    for (int y = 50; y < 53; y++) {
        for (int x = ox + lx - 2; x < ox + lx + 9; x++) SetPixel(surf, x, y, skinS);
        for (int x = ox + rx - 2; x < ox + rx + 9; x++) SetPixel(surf, x, y, skinS);
    }
}

SDL_Texture* CreatePlayerSheetTexture(SDL_Renderer* renderer) {
    int fw = 40, fh = 54, nframes = 4;
    SDL_Surface* surf = SDL_CreateSurface(fw * nframes, fh, SDL_PIXELFORMAT_RGBA8888);
    if (!surf) return nullptr;
    SDL_LockSurface(surf);

    uint32_t skin   = RGBA(0xFF, 0xDB, 0xAC, 0xFF);
    uint32_t skinS  = RGBA(0xE8, 0xC4, 0x90, 0xFF);
    uint32_t hair   = RGBA(0x5C, 0x3A, 0x1E, 0xFF);
    uint32_t hairL  = RGBA(0x7A, 0x52, 0x2E, 0xFF);
    uint32_t shirt  = RGBA(0x44, 0x88, 0xFF, 0xFF);
    uint32_t shirtD = RGBA(0x33, 0x6E, 0xCC, 0xFF);
    uint32_t pants  = RGBA(0x4A, 0x4A, 0x6A, 0xFF);
    uint32_t shoes  = RGBA(0x66, 0x44, 0x22, 0xFF);
    uint32_t whiteE = RGBA(0xFF, 0xFF, 0xFF, 0xFF);
    uint32_t pupil  = RGBA(0x22, 0x22, 0x22, 0xFF);
    uint32_t mouth  = RGBA(0xCC, 0x66, 0x66, 0xFF);
    uint32_t blush  = RGBA(0xFF, 0xAA, 0xAA, 0x44);

    DrawPlayerHead(surf, 0, skin, hair, hairL, whiteE, pupil, mouth, blush);
    DrawPlayerTorso(surf, 0, skin, skinS, shirt, shirtD);
    DrawPlayerLegs(surf, 0, pants, shoes, skinS, 0, 0);

    DrawPlayerHead(surf, 40, skin, hair, hairL, whiteE, pupil, mouth, blush);
    DrawPlayerTorso(surf, 40, skin, skinS, shirt, shirtD);
    DrawPlayerLegs(surf, 40, pants, shoes, skinS, 3, -3);

    DrawPlayerHead(surf, 80, skin, hair, hairL, whiteE, pupil, mouth, blush);
    DrawPlayerTorso(surf, 80, skin, skinS, shirt, shirtD);
    DrawPlayerLegs(surf, 80, pants, shoes, skinS, 1, -1);

    DrawPlayerHead(surf, 120, skin, hair, hairL, whiteE, pupil, mouth, blush);
    DrawPlayerTorso(surf, 120, skin, skinS, shirt, shirtD);
    DrawPlayerLegs(surf, 120, pants, shoes, skinS, -3, 3);

    SDL_UnlockSurface(surf);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_DestroySurface(surf);
    return tex;
}

SDL_Texture* CreateGroundTexture(SDL_Renderer* renderer) {
    SDL_Surface* surf = SDL_CreateSurface(1280, 40, SDL_PIXELFORMAT_RGBA8888);
    if (!surf) return nullptr;
    SDL_LockSurface(surf);
    uint32_t grass    = RGBA(0x44, 0xBB, 0x44, 0xFF);
    uint32_t darkG    = RGBA(0x33, 0x99, 0x33, 0xFF);
    uint32_t grassHi  = RGBA(0x5C, 0xCC, 0x5C, 0xFF);
    uint32_t dirt     = RGBA(0x8B, 0x6B, 0x4A, 0xFF);
    uint32_t darkD    = RGBA(0x6B, 0x4A, 0x2A, 0xFF);

    for (int y = 0; y < 40; y++) {
        bool isGrass = y < 6;
        for (int x = 0; x < 1280; x++) {
            uint32_t c;
            if (isGrass) {
                int pat = (x + y * 3) % 6;
                if (pat < 2) c = grass;
                else if (pat < 4) c = darkG;
                else c = grassHi;
            } else {
                int pat = (x + y * 7) % 8;
                if (pat < 5) c = dirt;
                else c = darkD;
                if (pat == 5 && (x % 13 == 3))
                    c = RGBA(0x99, 0x79, 0x58, 0xFF);
            }
            SetPixel(surf, x, y, c);
        }
    }
    SDL_UnlockSurface(surf);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_DestroySurface(surf);
    return tex;
}

SDL_Texture* CreateObstacleTexture(SDL_Renderer* renderer) {
    SDL_Surface* surf = SDL_CreateSurface(32, 48, SDL_PIXELFORMAT_RGBA8888);
    if (!surf) return nullptr;
    SDL_LockSurface(surf);
    uint32_t body   = RGBA(0xDD, 0x44, 0x44, 0xFF);
    uint32_t dark   = RGBA(0xAA, 0x22, 0x22, 0xFF);
    uint32_t spike  = RGBA(0xFF, 0x77, 0x55, 0xFF);
    uint32_t base   = RGBA(0x8B, 0x5B, 0x3A, 0xFF);

    for (int y = 42; y < 48; y++)
        for (int x = 6; x < 26; x++)
            SetPixel(surf, x, y, base);

    for (int y = 0; y < 44; y++) {
        int halfW = 4 + (44 - y) / 4;
        if (halfW < 2) halfW = 2;
        for (int x = 16 - halfW; x < 16 + halfW; x++) {
            if (x < 0 || x >= 32) continue;
            uint32_t c = ((x + y) % 4 == 0) ? dark : body;
            if (x >= 14 && x <= 17) c = dark;
            SetPixel(surf, x, y, c);
        }
    }
    for (int y = 4; y < 36; y += 6) {
        int leftX  = 16 - 4 - (44 - y) / 4;
        int rightX = 16 + 4 + (44 - y) / 4;
        for (int dy = -2; dy <= 2; dy++) {
            if (leftX > 0)  SetPixel(surf, leftX - 2, y + dy, spike);
            if (rightX < 32) SetPixel(surf, rightX + 2, y + dy, spike);
        }
    }
    for (int y = 0; y < 6; y++)
        SetPixel(surf, 16, y, spike);

    SDL_UnlockSurface(surf);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_DestroySurface(surf);
    return tex;
}

SDL_Texture* CreateCoinTexture(SDL_Renderer* renderer) {
    SDL_Surface* surf = SDL_CreateSurface(16, 16, SDL_PIXELFORMAT_RGBA8888);
    if (!surf) return nullptr;
    SDL_LockSurface(surf);
    uint32_t gold   = RGBA(0xFF, 0xDD, 0x00, 0xFF);
    uint32_t bright = RGBA(0xFF, 0xEE, 0x66, 0xFF);
    uint32_t dark   = RGBA(0xAA, 0x88, 0x00, 0xFF);
    uint32_t shine  = RGBA(0xFF, 0xFF, 0xCC, 0xAA);

    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            int dx = x - 8, dy = y - 8;
            int d2 = dx * dx + dy * dy;
            if (d2 > 60) continue;
            uint32_t c;
            if (d2 < 16) c = bright;
            else if (d2 < 36) c = gold;
            else c = dark;
            SetPixel(surf, x, y, c);
        }
    }
    SetPixel(surf, 5, 4, shine); SetPixel(surf, 6, 4, shine);
    SetPixel(surf, 5, 5, shine);

    SDL_UnlockSurface(surf);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_DestroySurface(surf);
    return tex;
}

} // namespace engine::application
