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
                                        0,
                                        0,
                                        17,
                                        17,
                                        17,
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
            switch (static_cast<NPCType>(entityTypeId)) {
                case NPCType::GOBLIN: {
                    struct SkinCfg {
                        const char* sheet;
                        int x, y, w, h, scale, strideX, strideY, cols;
                    };
                    static const SkinCfg skins[] = {
                            {"monsters/goblin1.png", 2, 2, 22, 48, 120, 24, 51, 6},
                            {"monsters/goblin2.png", 3, 3, 20, 29, 132, 25, 32, 8},
                    };
                    const SkinCfg& s = skins[entityId % (sizeof(skins) / sizeof(skins[0]))];
                    return EntitySprite{.bodySheet = s.sheet,
                                        .drawHead = false,
                                        .bodySrcX = s.x,
                                        .bodySrcY = s.y,
                                        .bodySrcW = s.w,
                                        .bodySrcH = s.h,
                                        .bodyScale = s.scale,
                                        .customGrid = true,
                                        .bodyStrideX = s.strideX,
                                        .bodyStrideY = s.strideY,
                                        .bodyCols = s.cols};
                }
                case NPCType::ORC: {
                    struct SkinCfg {
                        const char* sheet;
                        int x, y, w, h, scale, strideX, strideY, cols;
                    };
                    static const SkinCfg skins[] = {
                            {"monsters/orco1.png", 2, 1, 22, 58, 125, 26, 69, 6},
                            {"monsters/orco2.png", 1, 3, 22, 49, 125, 22, 55, 6},
                    };
                    const SkinCfg& s = skins[entityId % (sizeof(skins) / sizeof(skins[0]))];
                    return EntitySprite{.bodySheet = s.sheet,
                                        .drawHead = false,
                                        .bodySrcX = s.x,
                                        .bodySrcY = s.y,
                                        .bodySrcW = s.w,
                                        .bodySrcH = s.h,
                                        .bodyScale = s.scale,
                                        .customGrid = true,
                                        .bodyStrideX = s.strideX,
                                        .bodyStrideY = s.strideY,
                                        .bodyCols = s.cols};
                }
                case NPCType::ZOMBIE: {
                    struct SkinCfg {
                        const char* sheet;
                        int x, y, w, h, scale, strideX, strideY, cols;
                    };
                    static const SkinCfg skins[] = {
                            {"monsters/zombie1.png", 5, 2, 20, 44, 130, 23, 47, 8},
                            {"monsters/zombie2.png", 3, 0, 20, 51, 130, 25, 53, 6},
                    };
                    const SkinCfg& s = skins[entityId % (sizeof(skins) / sizeof(skins[0]))];
                    return EntitySprite{.bodySheet = s.sheet,
                                        .drawHead = false,
                                        .headSheet = "420.png",
                                        .bodySrcX = s.x,
                                        .bodySrcY = s.y,
                                        .bodySrcW = s.w,
                                        .bodySrcH = s.h,
                                        .bodyScale = s.scale,
                                        .customGrid = true,
                                        .bodyStrideX = s.strideX,
                                        .bodyStrideY = s.strideY,
                                        .bodyCols = s.cols};
                }
                case NPCType::SPIDER: {
                    struct SkinCfg {
                        const char* sheet;
                        int x, y, w, h, scale, strideX, strideY, cols;
                    };
                    static const SkinCfg skins[] = {
                            {"monsters/spider1.png", 18, 42, 75, 54, 65, 95, 96, 5},
                            {"monsters/spider2.png", 1, 0, 51, 37, 96, 54, 35, 4},
                            {"monsters/spider3.png", 0, 8, 60, 39, 81, 64, 66, 5},
                    };
                    const SkinCfg& s = skins[entityId % (sizeof(skins) / sizeof(skins[0]))];
                    return EntitySprite{.bodySheet = s.sheet,
                                        .drawHead = false,
                                        .bodySrcX = s.x,
                                        .bodySrcY = s.y,
                                        .bodySrcW = s.w,
                                        .bodySrcH = s.h,
                                        .bodyScale = s.scale,
                                        .customGrid = true,
                                        .bodyStrideX = s.strideX,
                                        .bodyStrideY = s.strideY,
                                        .bodyCols = s.cols};
                }
                case NPCType::GOLEM: {
                    struct SkinCfg {
                        const char* sheet;
                        int x, y, w, h, scale, strideX, strideY, cols;
                    };
                    static const SkinCfg skins[] = {
                            {"monsters/golem1.png", 4, 17, 46, 72, 100, 57, 99, 6},
                            {"monsters/golem2.png", 11, 14, 120, 155, 40, 161, 159, 6},
                            {"monsters/golem3.png", 11, 21, 130, 170, 40, 161, 198, 6},
                    };
                    const SkinCfg& s = skins[entityId % (sizeof(skins) / sizeof(skins[0]))];
                    return EntitySprite{.bodySheet = s.sheet,
                                        .drawHead = false,
                                        .bodySrcX = s.x,
                                        .bodySrcY = s.y,
                                        .bodySrcW = s.w,
                                        .bodySrcH = s.h,
                                        .bodyScale = s.scale,
                                        .customGrid = true,
                                        .bodyStrideX = s.strideX,
                                        .bodyStrideY = s.strideY,
                                        .bodyCols = s.cols};
                }
                case NPCType::SKELETON: {
                    struct SkinCfg {
                        const char* sheet;
                        int x, y, w, h, scale, strideX, strideY, cols;
                    };
                    static const SkinCfg skins[] = {
                            {"monsters/skeleton1.png", 5, 3, 16, 48, 120, 25, 52, 6},
                            {"monsters/skeleton2.png", 0, 1, 24, 50, 120, 26, 52, 6},
                            {"monsters/skeleton3.png", 4, 2, 21, 45, 120, 25, 47, 6},
                    };
                    const SkinCfg& s = skins[entityId % (sizeof(skins) / sizeof(skins[0]))];
                    return EntitySprite{.bodySheet = s.sheet,
                                        .drawHead = false,
                                        .bodySrcX = s.x,
                                        .bodySrcY = s.y,
                                        .bodySrcW = s.w,
                                        .bodySrcH = s.h,
                                        .bodyScale = s.scale,
                                        .customGrid = true,
                                        .bodyStrideX = s.strideX,
                                        .bodyStrideY = s.strideY,
                                        .bodyCols = s.cols};
                }
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
