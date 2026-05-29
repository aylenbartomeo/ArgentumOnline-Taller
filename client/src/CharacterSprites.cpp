#include "CharacterSprites.h"

EntitySprite spriteForEntity(EntityType type) {
    switch (type) {
        case EntityType::PLAYER:
            return EntitySprite{"1500.png", true, 6};
        case EntityType::NPC:
            return EntitySprite{"1200.png", true, 15};
        case EntityType::MONSTER:
            return EntitySprite{"1200.png", false, 0};
    }
    return EntitySprite{"1500.png", true, 6};
}
