#ifndef NPC_VISUALS_H
#define NPC_VISUALS_H

#include <cmath>
#include <string>

#include <SDL2pp/SDL2pp.hh>

#include "../animation/CharacterSprites.h"
#include "../rendering/Viewport.h"

// ─── NpcVisuals.h ─────────────────────────────────────────────────────────────
// Centraliza toda la información visual de los NPCs ciudadanos:
// - hojas de sprites y recortes de cabeza por tipo
// - codificación/decodificación del "fakeId" por coordenadas
// - dibujo del anillo de selección (elipse)

namespace NpcVisuals {

// ─── Sprites ──────────────────────────────────────────────────────────────────

inline const char* bodySheet(const std::string& type) {
    if (type == "merchant")
        return "1077.png";
    if (type == "banker")
        return "1071.png";
    if (type == "priest")
        return "1910.png";
    return "1200.png";
}

inline FrameRect headRect(const std::string& type) {
    if (type == "merchant")
        return FrameRect{115, 13, 13, 15};
    if (type == "banker")
        return FrameRect{142, 13, 13, 15};
    if (type == "priest")
        return FrameRect{170, 13, 11, 15};
    return FrameRect{6, 13, 13, 15};
}

inline std::string displayName(const std::string& type) {
    if (type == "merchant")
        return "Comerciante";
    if (type == "banker")
        return "Banquero";
    if (type == "priest")
        return "Sacerdote";
    return type;
}

// ─── FakeId por coordenadas ───────────────────────────────────────────────────

inline uint32_t encodeId(int x, int y) {
    return (static_cast<uint32_t>(x) << 16) | static_cast<uint32_t>(y);
}

inline bool matchesFakeId(uint32_t fakeId, int x, int y) { return fakeId == encodeId(x, y); }

// ─── Anillo de selección ──────────────────────────────────────────────────────

constexpr double TAU = 6.283185307179586;
constexpr int SEGMENTS = 24;
constexpr int SHIFT_X = 3;
constexpr int ELLIPSE_OFFSET_Y = 4;

// Dibuja una elipse de selección en los pies del NPC/jugador.
inline void drawSelectionEllipse(SDL2pp::Renderer& renderer, int tilePixelX, int tilePixelY,
                                 int tileSize, Uint8 r, Uint8 g, Uint8 b) {
    renderer.SetDrawColor(r, g, b, 255);
    const int cx = tilePixelX + tileSize / 2 - SHIFT_X;
    const int cy = tilePixelY + tileSize - ELLIPSE_OFFSET_Y;

    for (int t = -1; t <= 1; ++t) {
        const int rx = tileSize / 2 - 2 + t;
        const int ry = tileSize / 5 + t;
        for (int i = 0; i < SEGMENTS; ++i) {
            const double a0 = TAU * i / SEGMENTS;
            const double a1 = TAU * (i + 1) / SEGMENTS;
            renderer.DrawLine(cx + static_cast<int>(rx * std::cos(a0)),
                              cy + static_cast<int>(ry * std::sin(a0)),
                              cx + static_cast<int>(rx * std::cos(a1)),
                              cy + static_cast<int>(ry * std::sin(a1)));
        }
    }
}

}  // namespace NpcVisuals

#endif
