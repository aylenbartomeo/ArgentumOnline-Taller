#include <gtest/gtest.h>

#include "CharacterSprites.h"

TEST(CharacterSpritesTest, PlayerUsesBodyWithHead) {
    EntitySprite s = spriteForEntity(EntityType::PLAYER);
    EXPECT_STREQ(s.bodySheet, "1500.png");
    EXPECT_TRUE(s.drawHead);
}

TEST(CharacterSpritesTest, NpcUsesAnotherBodyWithHead) {
    EntitySprite s = spriteForEntity(EntityType::NPC);
    EXPECT_STREQ(s.bodySheet, "1200.png");
    EXPECT_TRUE(s.drawHead);
}

TEST(CharacterSpritesTest, MonsterUsesAnotherBodyWithoutHead) {
    EntitySprite s = spriteForEntity(EntityType::MONSTER);
    EXPECT_STREQ(s.bodySheet, "1200.png");
    EXPECT_FALSE(s.drawHead);
}

TEST(CharacterSpritesTest, PlayerAndNpcUseDifferentBodies) {
    EXPECT_STRNE(spriteForEntity(EntityType::PLAYER).bodySheet,
                 spriteForEntity(EntityType::NPC).bodySheet);
}
