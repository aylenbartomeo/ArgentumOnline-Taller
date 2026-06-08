#include <gtest/gtest.h>

#include "../client/src/animation/CharacterSprites.h"

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

TEST(CharacterSpritesTest, MonsterSkeletonUses152Png) {
    EXPECT_STREQ(spriteForEntity(EntityType::MONSTER, 1238).bodySheet, "152.png");
}

TEST(CharacterSpritesTest, HumanoidMonstersDrawHead) {
    EXPECT_TRUE(spriteForEntity(EntityType::MONSTER, 1800).drawHead);
    EXPECT_TRUE(spriteForEntity(EntityType::MONSTER, 1875).drawHead);
    EXPECT_TRUE(spriteForEntity(EntityType::MONSTER, 1892).drawHead);
}

TEST(CharacterSpritesTest, BeastMonstersHaveNoHead) {
    EXPECT_FALSE(spriteForEntity(EntityType::MONSTER, 1052).drawHead);
    EXPECT_FALSE(spriteForEntity(EntityType::MONSTER, 1140).drawHead);
    EXPECT_FALSE(spriteForEntity(EntityType::MONSTER, 1238).drawHead);
}

TEST(CharacterSpritesTest, GreenMonstersUseGreenHeadSheets) {
    EXPECT_STREQ(spriteForEntity(EntityType::MONSTER, 1875).headSheet, "430.png");
    EXPECT_STREQ(spriteForEntity(EntityType::MONSTER, 1800).headSheet, "422.png");
    EXPECT_STREQ(spriteForEntity(EntityType::MONSTER, 1892).headSheet, "420.png");
}

TEST(CharacterSpritesTest, MonsterHeadUsesDownRow) {
    EntitySprite s = spriteForEntity(EntityType::MONSTER, 1892);
    EXPECT_EQ(s.headSrcX, 89);
    EXPECT_EQ(s.headSrcY, 13);
    EXPECT_EQ(s.headSrcW, 11);
    EXPECT_EQ(s.headSrcH, 15);
}

TEST(CharacterSpritesTest, HumanoidBodyUsesStandardCrop) {
    EntitySprite s = spriteForEntity(EntityType::MONSTER, 1800);
    EXPECT_EQ(s.bodySrcX, 2);
    EXPECT_EQ(s.bodySrcY, 4);
    EXPECT_EQ(s.bodySrcW, 24);
    EXPECT_EQ(s.bodySrcH, 44);
}

TEST(CharacterSpritesTest, BeastsUseFullFigureBody) {
    EXPECT_EQ(spriteForEntity(EntityType::MONSTER, 1140).bodySrcH, 30);
    EXPECT_EQ(spriteForEntity(EntityType::MONSTER, 1238).bodySrcH, 38);
    EXPECT_NE(spriteForEntity(EntityType::MONSTER, 1052).bodySrcH, 44);
}

TEST(CharacterSpritesTest, BeastBodyStaysWithinDownRow) {
    EntitySprite s = spriteForEntity(EntityType::MONSTER, 1052);
    EXPECT_LT(s.bodySrcY + s.bodySrcH, 48);
}

TEST(CharacterSpritesTest, PlayerHeadUsesFirstColumn) {
    EntitySprite s = spriteForEntity(EntityType::PLAYER, 0);
    EXPECT_EQ(s.headSrcX, 6);
    EXPECT_EQ(s.headSrcY, 13);
}
