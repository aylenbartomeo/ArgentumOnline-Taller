#include "CharacterSprites.h"

#include "common/utils/types.h"

EntitySprite spriteForEntity(EntityType type, uint8_t entityTypeId, uint32_t entityId) {
    switch (type) {
        case EntityType::PLAYER:
            return EntitySprite{"1500.png", true, 6, "420.png", 6, 13, 13, 15, 2, 4, 24, 44};
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
    const int row = rowForFacing(facing);
    return FrameRect{sprite.bodySrcX + c * sprite.bodyStrideX,
                     sprite.bodySrcY + row * sprite.bodyStrideY, sprite.bodySrcW, sprite.bodySrcH};
}
