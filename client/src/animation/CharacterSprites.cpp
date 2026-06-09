#include "CharacterSprites.h"

EntitySprite spriteForEntity(EntityType type, uint16_t spriteId) {
    switch (type) {
        case EntityType::PLAYER:
            return EntitySprite{"1500.png", true, 6, "420.png", 6, 13, 13, 15, 2, 4, 24, 44};
        case EntityType::NPC:
            return EntitySprite{"1200.png", true, 15, "420.png", 6, 13, 13, 15, 2, 4, 24, 44};
        case EntityType::MONSTER:
            switch (spriteId) {
                case 1800:
                    return EntitySprite{"1800.png", true, 6, "422.png", 195, 14,
                                        15,         14,   2, 4,         24,  44};
                case 1875:
                    return EntitySprite{"1875.png", true, 15, "430.png", 6,  16,
                                        15,         13,   2,  4,         24, 44};
                case 1892:
                    return EntitySprite{"1892.png", true, 6, "420.png", 89, 13,
                                        11,         15,   2, 4,         24, 44};
                case 1052:
                    return EntitySprite{"1052.png", false, 0, "420.png", 6,  13,
                                        13,         15,    4, 5,         19, 39};
                case 1140:
                    return EntitySprite{"1140.png", false, 0, "420.png", 6,  13,
                                        13,         15,    3, 16,        21, 30};
                case 1238:
                    return EntitySprite{"152.png", false, 0, "420.png", 6,  13, 13,
                                        15,        4,     4, 22,        38, 140};
                default:
                    return EntitySprite{"1200.png", false, 0, "420.png", 6,  13,
                                        13,         15,    2, 4,         24, 44};
            }
    }
    return EntitySprite{"1500.png", true, 6, "420.png", 6, 13, 13, 15, 2, 4, 24, 44};
}
