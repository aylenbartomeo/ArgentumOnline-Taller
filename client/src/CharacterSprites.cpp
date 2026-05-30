#include "CharacterSprites.h"

EntitySprite spriteForEntity(EntityType type, uint16_t spriteId) {
    switch (type) {
        case EntityType::PLAYER:
            return EntitySprite{"1500.png", true, 6};
        case EntityType::NPC:
            return EntitySprite{"1200.png", true, 15};
        case EntityType::MONSTER:
            switch (spriteId) {
                case 1800:
                    return EntitySprite{"1800.png", false, 0};
                case 1875:
                    return EntitySprite{"1875.png", false, 0};
                case 1892:
                    return EntitySprite{"1892.png", false, 0};
                case 1052:
                    return EntitySprite{"1052.png", false, 0};
                case 1140:
                    return EntitySprite{"1140.png", false, 0};
                case 1238:
                    return EntitySprite{"1238.png", false, 0};
                default:
                    return EntitySprite{"1200.png", false, 0};
            }
    }
    return EntitySprite{"1500.png", true, 6};
}
