#ifndef SERVER_SRC_MODEL_ITEMS_ITEMCONSTANTS_H
#define SERVER_SRC_MODEL_ITEMS_ITEMCONSTANTS_H

#include <cstdint>

// IDs reservados para items especiales
static constexpr uint32_t GOLD_ITEM_ID = 1;

static constexpr uint32_t ITEM_VARA_FRESNO = 2020;
static constexpr uint32_t ITEM_FLAUTA_ELFICA = 2021;
static constexpr uint32_t ITEM_BACULO_NUDOSO = 2022;
static constexpr uint32_t ITEM_BACULO_ENGARZADO = 2023;
static constexpr uint32_t ITEM_ARCO_SIMPLE = 2010;
static constexpr uint32_t ITEM_ARCO_COMPUESTO = 2011;

// Sprite IDs para proyectiles
static constexpr uint16_t SPRITE_ARROW = 200;
static constexpr uint16_t SPRITE_MAGIC_ARROW = 201;
static constexpr uint16_t SPRITE_MISSILE = 202;
static constexpr uint16_t SPRITE_EXPLOSION = 203;

// Curación de flauta élfica (HP fijo por hechizo):
static constexpr uint16_t FLAUTA_HEAL_AMOUNT = 50;

#endif  // SERVER_SRC_MODEL_ITEMS_ITEMCONSTANTS_H
