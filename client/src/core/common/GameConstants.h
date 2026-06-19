#ifndef GAME_CONSTANTS_H
#define GAME_CONSTANTS_H

#include <cstdint>

namespace GameConstants {
// Configuración Global y Ventana
constexpr int TILE_SIZE = 32;
constexpr int WINDOW_WIDTH = 1024;
constexpr int WINDOW_HEIGHT = 768;
constexpr const char* RESOURCES_DIR = "resources/";

// Cámara
constexpr int VIEW_X = 0;
constexpr int VIEW_Y = 29;
constexpr int VIEW_W = 757;
constexpr int VIEW_H = 736;

// Constantes compartidas de Renderizado (usadas en Entity y World Renderer)
constexpr int CHARACTER_DRAW_H = TILE_SIZE * 3 / 2;
constexpr const char* HEAD_SHEET = "420.png";
constexpr int HEAD_DRAW_W = 18;
constexpr int HEAD_DRAW_H = 20;

constexpr uint16_t MAX_LEVEL = 99;
}  // namespace GameConstants

#endif
