#include "CharacterSprites.h"

#include "common/utils/types.h"

EntitySprite spriteForEntity(EntityType type, uint8_t entityTypeId, uint32_t entityId,
                             uint8_t stateId) {
    switch (type) {
        case EntityType::PLAYER: {
            switch (entityTypeId) {
                case 0:  // Humano
                    return EntitySprite{"race/human/human-body.png",
                                        true,
                                        15,
                                        "race/human/human-head.png",
                                        0,
                                        0,
                                        16,
                                        16,
                                        16,
                                        3,
                                        5,
                                        20,
                                        39,
                                        100,
                                        false,
                                        25,
                                        45,
                                        6};
                case 1:  // Elfo
                    return EntitySprite{"race/elf/elf-body.png",
                                        true,
                                        14,
                                        "race/elf/elf-head.png",
                                        0,
                                        0,
                                        16,
                                        16,
                                        16,
                                        3,
                                        5,
                                        20,
                                        39,
                                        100,
                                        false,
                                        25,
                                        45,
                                        6};
                case 2:  // Enano
                    return EntitySprite{"race/dwarf/dwarf-body.png",
                                        true,
                                        28,
                                        "race/dwarf/dwarf-head.png",
                                        2,
                                        2,
                                        13,
                                        19,
                                        19,
                                        3,
                                        12,
                                        14,
                                        26,
                                        100,
                                        false,
                                        20,
                                        36,
                                        6};
                case 3:  // Gnomo
                    return EntitySprite{"race/gnome/gnome-body.png",
                                        true,
                                        28,
                                        "race/gnome/gnome-head.png",
                                        2,
                                        2,
                                        10,
                                        11,
                                        13,
                                        4,
                                        12,
                                        13,
                                        23,
                                        100,
                                        false,
                                        20,
                                        36,
                                        6};
                default:  // Fallback por defecto si viene otro ID
                    return EntitySprite{"1500.png", true, 6, "420.png", 6,  13,
                                        13,         15,   2, 4,         24, 44};
            }
        }
        case EntityType::NPC:
            return EntitySprite{"1200.png", true, 15, "420.png", 6, 13, 13, 15, 2, 4, 24, 44};
        case EntityType::MONSTER: {
            // int goblinOrcHeadX = 6 + (entityId % 18) * 27;
            switch (static_cast<NPCType>(entityTypeId)) {
                case NPCType::GOBLIN:
                    return EntitySprite{.bodySheet = "4015.png",
                                        .drawHead = false,
                                        .bodySrcX = 2,
                                        .bodySrcY = 2,
                                        .bodySrcW = 22,
                                        .bodySrcH = 48,
                                        .bodyScale = 120,
                                        .customGrid = true,
                                        .bodyStrideX = 24,
                                        .bodyStrideY = 51,
                                        .bodyCols = 6};
                case NPCType::ORC:
                    return EntitySprite{.bodySheet = "4017.png",
                                        .drawHead = false,
                                        .bodySrcX = 2,
                                        .bodySrcY = 1,
                                        .bodySrcW = 22,
                                        .bodySrcH = 58,
                                        .bodyScale = 125,
                                        .customGrid = true,
                                        .bodyStrideX = 26,
                                        .bodyStrideY = 69,
                                        .bodyCols = 6};
                case NPCType::ZOMBIE:
                    return EntitySprite{.bodySheet = "4044.png",
                                        .drawHead = false,
                                        .headSheet = "420.png",
                                        .bodySrcX = 5,
                                        .bodySrcY = 2,
                                        .bodySrcW = 20,
                                        .bodySrcH = 44,
                                        .bodyScale = 130,
                                        .customGrid = true,
                                        .bodyStrideX = 23,
                                        .bodyStrideY = 47,
                                        .bodyCols = 8};
                case NPCType::SPIDER:
                    return EntitySprite{.bodySheet = "4151.png",
                                        .drawHead = false,
                                        .bodySrcX = 18,
                                        .bodySrcY = 42,
                                        .bodySrcW = 75,
                                        .bodySrcH = 54,
                                        .bodyScale = 65,
                                        .customGrid = true,
                                        .bodyStrideX = 95,
                                        .bodyStrideY = 96,
                                        .bodyCols = 5};
                case NPCType::GOLEM:
                    return EntitySprite{.bodySheet = "4091.png",
                                        .drawHead = false,
                                        .bodySrcX = 4,
                                        .bodySrcY = 17,
                                        .bodySrcW = 46,
                                        .bodySrcH = 72,
                                        .bodyScale = 100,
                                        .customGrid = true,
                                        .bodyStrideX = 57,
                                        .bodyStrideY = 99,
                                        .bodyCols = 6};
                case NPCType::SKELETON:
                    return EntitySprite{.bodySheet = "4079.png",
                                        .drawHead = false,
                                        .bodySrcX = 5,
                                        .bodySrcY = 3,
                                        .bodySrcW = 16,
                                        .bodySrcH = 48,
                                        .bodyScale = 120,
                                        .customGrid = true,
                                        .bodyStrideX = 25,
                                        .bodyStrideY = 52,
                                        .bodyCols = 6};
                case NPCType::BOSS_BALROG:
                    return EntitySprite{.bodySheet = "bosses/Balrog Infernal.png",
                                        .drawHead = false,
                                        .bodySrcX = 0,
                                        .bodySrcY = 0,
                                        .bodySrcW = 84,
                                        .bodySrcH = 130,
                                        .bodyScale = 100,
                                        .customGrid = true,
                                        .bodyStrideX = 84,
                                        .bodyStrideY = 130,
                                        .bodyCols = 4,
                                        .rowOrder = 1};
                case NPCType::BOSS_TITAN:
                    return EntitySprite{.bodySheet = "bosses/Titan de Piedra.png",
                                        .drawHead = false,
                                        .bodySrcX = 0,
                                        .bodySrcY = 0,
                                        .bodySrcW = 58,
                                        .bodySrcH = 100,
                                        .bodyScale = 100,
                                        .customGrid = true,
                                        .bodyStrideX = 58,
                                        .bodyStrideY = 100,
                                        .bodyCols = 4,
                                        .rowOrder = 1};
                case NPCType::BOSS_ARACNE:
                    return EntitySprite{.bodySheet = "bosses/Aracne Abismal.png",
                                        .drawHead = false,
                                        .bodySrcX = 0,
                                        .bodySrcY = 0,
                                        .bodySrcW = 67,
                                        .bodySrcH = 120,
                                        .bodyScale = 100,
                                        .customGrid = true,
                                        .bodyStrideX = 67,
                                        .bodyStrideY = 120,
                                        .bodyCols = 6,
                                        .rowOrder = 1};
                case NPCType::BOSS_COLOSO:
                    return EntitySprite{.bodySheet = "bosses/Coloso de Magma.png",
                                        .drawHead = false,
                                        .bodySrcX = 0,
                                        .bodySrcY = 0,
                                        .bodySrcW = 152,
                                        .bodySrcH = 162,
                                        .bodyScale = 100,
                                        .customGrid = true,
                                        .bodyStrideX = 152,
                                        .bodyStrideY = 162,
                                        .bodyCols = 4,
                                        .rowOrder = 1};
                default:
                    return EntitySprite{"1200.png", false, 0, "420.png", 6,  13,
                                        13,         15,    2, 4,         24, 44};
            }
        }
    }
    return EntitySprite{"1500.png", true, 6, "420.png", 6, 13, 13, 15, 2, 4, 24, 44};
}

FrameRect bodyFrameRectFor(const EntitySprite& sprite, Movement facing, int col) {
    int c = col < 0 ? 0 : col;
    if (c >= sprite.bodyCols) {
        c = sprite.bodyCols - 1;
    }
    int row = 0;
    int srcW = sprite.bodySrcW;
    int srcH = sprite.bodySrcH;
    int strideX = sprite.bodyStrideX;
    int offsetX = 0;
    int offsetY = 0;

    if (sprite.rowOrder == 1) {
        switch (facing) {
            case Movement::DOWN:
                row = 0;
                break;
            case Movement::UP:
                row = 1;
                break;
            case Movement::LEFT:
                row = 2;
                break;
            case Movement::RIGHT:
                row = 3;
                break;
        }
    } else {
        row = rowForFacing(facing);
    }

    int x = sprite.bodySrcX + offsetX + c * strideX;
    int y = sprite.bodySrcY + offsetY + row * sprite.bodyStrideY;

    return FrameRect{x, y, srcW, srcH};
}
