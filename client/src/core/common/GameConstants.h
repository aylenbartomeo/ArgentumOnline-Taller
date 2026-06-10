#ifndef GAME_CONSTANTS_H
#define GAME_CONSTANTS_H

#include <cstdint>

namespace GameConstants {

constexpr int TILE_SIZE = 32;
constexpr int WINDOW_WIDTH = 1024;
constexpr int WINDOW_HEIGHT = 768;
constexpr int VIEW_X = 0;
constexpr int VIEW_Y = 29;
constexpr int VIEW_W = 757;
constexpr int VIEW_H = 736;

constexpr const char* HUD_FONT_PATH = "resources/fonts/DejaVuSans.ttf";
constexpr const char* CHAT_FONT_PATH = "resources/fonts/DejaVuSans.ttf";
constexpr const char* RESOURCES_DIR = "resources/";

constexpr uint32_t MOVE_INTERVAL_MS = 200;

// Characters
constexpr int CHARACTER_FRAME_X = 2;
constexpr int CHARACTER_FRAME_Y = 4;
constexpr int CHARACTER_FRAME_W = 24;
constexpr int CHARACTER_FRAME_H = 44;
constexpr int CHARACTER_DRAW_H = TILE_SIZE * 3 / 2;

// Ellipse marker
constexpr double TAU = 6.283185307179586;
constexpr int MARKER_SEGMENTS = 24;
constexpr int MARKER_SHIFT_X = 3;

// Head
constexpr const char* HEAD_SHEET = "420.png";
constexpr int HEAD_DRAW_W = 18;
constexpr int HEAD_DRAW_H = 20;

// Health bar
constexpr const char* HEALTHBAR_SHEET = "en_barradevida.bmp";

// Skull
constexpr const char* SKULL_SHEET = "106.png";

// Default FX
constexpr const char* FX_SHEET = "19052.png";
constexpr int FX_FRAME_W = 64;
constexpr int FX_FRAME_H = 96;
constexpr int FX_COLS = 8;
constexpr int FX_FRAME_COUNT = 7;
constexpr uint32_t FX_FRAME_DUR_MS = 50;
constexpr int FX_DRAW_W = TILE_SIZE * 3 / 2;
constexpr int FX_DRAW_H = FX_DRAW_W * FX_FRAME_H / FX_FRAME_W;
constexpr int ATTACK_RANGE_TILES = 1;

// Sword FX
constexpr const char* SWORD_FX_SHEET = "2101.png";
constexpr int SWORD_FRAME_W = 32;
constexpr int SWORD_FRAME_H = 32;
constexpr int SWORD_FRAME_COLS = 5;
constexpr int SWORD_FRAME_COUNT = 21;
constexpr uint32_t SWORD_FRAME_DUR_MS = 40;
constexpr int SWORD_DRAW_W = TILE_SIZE * 2;
constexpr int SWORD_DRAW_H = TILE_SIZE * 2;
constexpr uint32_t SWORD_WEAPON_ID = 2000;

// Ground sheet
constexpr const char* GROUND_SHEET = "5108.png";
constexpr int GROUND_SRC_X = 416;
constexpr int GROUND_SRC_Y = 384;
constexpr int DARK_GROUND_SRC_X = 512;
constexpr int DARK_GROUND_SRC_Y = 480;
constexpr int GROUND_TILE = 32;

// Projectiles (generic)
constexpr int PROJ_DRAW_W = 64;
constexpr int PROJ_DRAW_H = 64;
constexpr int PROJ_FRAME_COLS = 8;
constexpr int PROJ_FRAME_SIZE = 64;
constexpr const char* PROJ_SHEET = "projectiles.png";

// Arrow
constexpr const char* ARROW_SHEET = "2046.png";
constexpr int ARROW_FRAME_COLS = 2;
constexpr int ARROW_FRAME_ROWS = 1;
constexpr int ARROW_FRAME_W = 32;
constexpr int ARROW_FRAME_H = 32;
constexpr int ARROW_SRC_Y = 224;
constexpr int ARROW_SRC_X0 = 32;
constexpr int ARROW_DRAW_W = 48;
constexpr int ARROW_DRAW_H = 48;
constexpr uint16_t ARROW_SPRITE_ID = 200;

// Staff projectile
constexpr const char* STAFF_PROJ_SHEET = "3492.png";
constexpr int STAFF_PROJ_FRAME_W = 128;
constexpr int STAFF_PROJ_FRAME_H = 128;
constexpr int STAFF_PROJ_COLS = 4;
constexpr int STAFF_PROJ_FRAMES = 16;
constexpr uint32_t STAFF_PROJ_DUR_MS = 60;
constexpr int STAFF_PROJ_DRAW = TILE_SIZE * 2;
constexpr uint16_t STAFF_SPRITE_ID = 203;

// Explosion impact
constexpr const char* EXPL_SHEET = "3471.png";
constexpr int EXPL_FRAME_W = 204;
constexpr int EXPL_FRAME_H = 204;
constexpr int EXPL_COLS = 5;
constexpr int EXPL_FRAMES = 22;
constexpr uint32_t EXPL_DUR_MS = 55;
constexpr int EXPL_DRAW = TILE_SIZE * 4;
constexpr uint32_t STAFF_WEAPON_ID = 203;

// Baculo Nudoso projectile
constexpr const char* NUDOSO_PROJ_SHEET = "3483.png";
constexpr int NUDOSO_PROJ_FRAME_W = 102;
constexpr int NUDOSO_PROJ_FRAME_H = 102;
constexpr int NUDOSO_PROJ_COLS = 5;
constexpr int NUDOSO_PROJ_FRAMES = 19;
constexpr uint32_t NUDOSO_PROJ_DUR_MS = 60;
constexpr int NUDOSO_PROJ_DRAW = TILE_SIZE * 2;
constexpr uint16_t NUDOSO_SPRITE_ID = 202;

// Baculo Nudoso impact
constexpr const char* NUDOSO_IMPACT_SHEET = "3534.png";
constexpr int NUDOSO_IMPACT_FRAME_W = 204;
constexpr int NUDOSO_IMPACT_FRAME_H = 204;
constexpr int NUDOSO_IMPACT_COLS = 5;
constexpr int NUDOSO_IMPACT_FRAMES = 25;
constexpr uint32_t NUDOSO_IMPACT_DUR_MS = 40;
constexpr int NUDOSO_IMPACT_DRAW = TILE_SIZE * 3;

// Vara de Fresno
constexpr uint32_t VARA_FRESNO_WEAPON_ID = 2020;
constexpr uint16_t VARA_SPRITE_ID = 201;
constexpr const char* FRESNO_IMPACT_SHEET = "3490.png";
constexpr int FRESNO_IMPACT_FRAME_W = 128;
constexpr int FRESNO_IMPACT_FRAME_H = 128;
constexpr int FRESNO_IMPACT_COLS = 4;
constexpr int FRESNO_IMPACT_FRAMES = 28;
constexpr uint32_t FRESNO_IMPACT_DUR_MS = 35;
constexpr int FRESNO_IMPACT_DRAW = TILE_SIZE * 3;

// Flauta Elfica
constexpr uint32_t FLAUTA_WEAPON_ID = 2021;
constexpr const char* FLAUTA_HEAL_SHEET = "3456.png";
constexpr int FLAUTA_HEAL_FRAME_W = 32;
constexpr int FLAUTA_HEAL_FRAME_H = 32;
constexpr int FLAUTA_HEAL_COLS = 5;
constexpr int FLAUTA_HEAL_FRAMES = 5;
constexpr uint32_t FLAUTA_HEAL_DUR_MS = 60;
constexpr int FLAUTA_HEAL_DRAW = TILE_SIZE * 2;

// Citizens
constexpr int CITIZEN_HEAD_OVERLAP = 6;

}  // namespace GameConstants

#endif
