#include <gtest/gtest.h>

#include "CharacterSprites.h"

TEST(CharacterSpritesTest, PlayerUsesBodyWithHead) {
    EntitySprite s = spriteForEntity(EntityType::PLAYER, 0);
    EXPECT_STREQ(s.bodySheet, "1500.png");
    EXPECT_TRUE(s.drawHead);
}

TEST(CharacterSpritesTest, NpcUsesAnotherBodyWithHead) {
    EntitySprite s = spriteForEntity(EntityType::NPC, 0);
    EXPECT_STREQ(s.bodySheet, "1200.png");
    EXPECT_TRUE(s.drawHead);
}

TEST(CharacterSpritesTest, MonsterWithoutKnownSpriteFallsBackToPlaceholder) {
    EntitySprite s = spriteForEntity(EntityType::MONSTER, 25);
    EXPECT_STREQ(s.bodySheet, "1200.png");
    EXPECT_FALSE(s.drawHead);
}

TEST(CharacterSpritesTest, PlayerAndNpcUseDifferentBodies) {
    EXPECT_STRNE(spriteForEntity(EntityType::PLAYER, 0).bodySheet,
                 spriteForEntity(EntityType::NPC, 0).bodySheet);
}

TEST(CharacterSpritesTest, MonsterGoblinUses1800) {
    EXPECT_STREQ(spriteForEntity(EntityType::MONSTER, 1800).bodySheet, "1800.png");
}

TEST(CharacterSpritesTest, MonsterOrcUses1875) {
    EXPECT_STREQ(spriteForEntity(EntityType::MONSTER, 1875).bodySheet, "1875.png");
}

TEST(CharacterSpritesTest, MonsterZombieUses1892) {
    EXPECT_STREQ(spriteForEntity(EntityType::MONSTER, 1892).bodySheet, "1892.png");
}

TEST(CharacterSpritesTest, MonsterSpiderUses1052) {
    EXPECT_STREQ(spriteForEntity(EntityType::MONSTER, 1052).bodySheet, "1052.png");
}

TEST(CharacterSpritesTest, MonsterGolemUses1140) {
    EXPECT_STREQ(spriteForEntity(EntityType::MONSTER, 1140).bodySheet, "1140.png");
}

TEST(CharacterSpritesTest, MonsterSkeletonUses1238) {
    EXPECT_STREQ(spriteForEntity(EntityType::MONSTER, 1238).bodySheet, "1238.png");
}
