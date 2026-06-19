#ifndef CATALOG_SPRITES_H
#define CATALOG_SPRITES_H

#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "../client/src/rendering/TextureManager.h"

#include "OverlayRegistry.h"
#include "SpawnCatalogs.h"

void drawOverlaySprite(SDL2pp::Renderer& renderer, TextureManager& textures, const OverlayDef& def,
                       int cellX, int cellY, int cellSize);
void drawMonsterSprite(SDL2pp::Renderer& renderer, TextureManager& textures,
                       const MonsterCatalogEntry& entry, int cellX, int cellY, int cellSize);
void drawCitizenSprite(SDL2pp::Renderer& renderer, TextureManager& textures,
                       const CitizenCatalogEntry& entry, int cellX, int cellY, int cellSize);
std::vector<int> itemOverlayIndices();

#endif
