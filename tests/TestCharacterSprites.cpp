#include <gtest/gtest.h>

#include "../client/src/animation/CharacterSprites.h"
#include "../common/utils/types.h"

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

TEST(CharacterSpritesTest, MonsterGoblinUses4015) {
    EXPECT_STREQ(spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::GOBLIN), 0)
                         .bodySheet,
                 "4015.png");
}

TEST(CharacterSpritesTest, MonsterOrcUses4017) {
    EXPECT_STREQ(
            spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::ORC), 0).bodySheet,
            "4017.png");
}

TEST(CharacterSpritesTest, MonsterZombieUses4044) {
    EXPECT_STREQ(spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::ZOMBIE), 0)
                         .bodySheet,
                 "4044.png");
}

TEST(CharacterSpritesTest, MonsterSpiderUses4151) {
    EXPECT_STREQ(spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::SPIDER), 0)
                         .bodySheet,
                 "4151.png");
}

TEST(CharacterSpritesTest, MonsterGolemUses4091) {
    EXPECT_STREQ(
            spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::GOLEM), 0).bodySheet,
            "4091.png");
}

TEST(CharacterSpritesTest, MonsterSkeletonUses4079) {
    EXPECT_STREQ(spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::SKELETON), 0)
                         .bodySheet,
                 "4079.png");
}

TEST(CharacterSpritesTest, ZombieHasNoHead) {
    EXPECT_FALSE(spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::ZOMBIE), 0)
                         .drawHead);
}

TEST(CharacterSpritesTest, BeastMonstersHaveNoHead) {
    EXPECT_FALSE(spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::SPIDER), 0)
                         .drawHead);
    EXPECT_FALSE(
            spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::GOLEM), 0).drawHead);
    EXPECT_FALSE(spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::SKELETON), 0)
                         .drawHead);
    EXPECT_FALSE(spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::GOBLIN), 0)
                         .drawHead);
    EXPECT_FALSE(
            spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::ORC), 0).drawHead);
}

TEST(CharacterSpritesTest, ZombieUsesHeadSheet) {
    EXPECT_STREQ(spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::ZOMBIE), 0)
                         .headSheet,
                 "420.png");
}

TEST(CharacterSpritesTest, ZombieGridUsesMeasuredStrides) {
    EntitySprite s = spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::ZOMBIE), 0);
    EXPECT_EQ(s.bodyStrideX, 23);
    EXPECT_EQ(s.bodyStrideY, 47);
}

TEST(CharacterSpritesTest, ZombieUsesCustomGrid) {
    EntitySprite s = spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::ZOMBIE), 0);
    EXPECT_TRUE(s.customGrid);
    EXPECT_EQ(s.bodyCols, 8);
}

TEST(CharacterSpritesTest, BeastsUseFullFigureBody) {
    EXPECT_EQ(
            spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::GOLEM), 0).bodySrcH,
            72);
    EXPECT_EQ(spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::SKELETON), 0)
                      .bodySrcH,
              48);
}

TEST(CharacterSpritesTest, SpiderUsesFiveColumnCustomGrid) {
    EntitySprite s = spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::SPIDER), 0);
    EXPECT_TRUE(s.customGrid);
    EXPECT_EQ(s.bodyCols, 5);
}

TEST(CharacterSpritesTest, PlayerHeadUsesFirstColumn) {
    EntitySprite s = spriteForEntity(EntityType::PLAYER, 0);
    EXPECT_EQ(s.headSrcX, 6);
    EXPECT_EQ(s.headSrcY, 13);
}

TEST(CharacterSpritesTest, BodyFrameRectForColZeroDownIsOrigin) {
    EntitySprite s{};
    s.bodySrcX = 10;
    s.bodySrcY = 20;
    s.bodySrcW = 22;
    s.bodySrcH = 48;
    s.bodyStrideX = 24;
    s.bodyStrideY = 51;
    s.bodyCols = 6;
    FrameRect r = bodyFrameRectFor(s, Movement::DOWN, 0);
    EXPECT_EQ(r.x, 10);
    EXPECT_EQ(r.y, 20);
    EXPECT_EQ(r.w, 22);
    EXPECT_EQ(r.h, 48);
}

TEST(CharacterSpritesTest, BodyFrameRectForColAddsStrideX) {
    EntitySprite s{};
    s.bodySrcX = 10;
    s.bodySrcY = 20;
    s.bodySrcW = 22;
    s.bodySrcH = 48;
    s.bodyStrideX = 24;
    s.bodyStrideY = 51;
    s.bodyCols = 6;
    FrameRect r = bodyFrameRectFor(s, Movement::DOWN, 2);
    EXPECT_EQ(r.x, 58);
    EXPECT_EQ(r.y, 20);
}

TEST(CharacterSpritesTest, BodyFrameRectForRightUsesRowThree) {
    EntitySprite s{};
    s.bodySrcX = 10;
    s.bodySrcY = 20;
    s.bodySrcW = 22;
    s.bodySrcH = 48;
    s.bodyStrideX = 24;
    s.bodyStrideY = 51;
    s.bodyCols = 6;
    FrameRect r = bodyFrameRectFor(s, Movement::RIGHT, 0);
    EXPECT_EQ(r.y, 173);
}

TEST(CharacterSpritesTest, BodyFrameRectForClampsCol) {
    EntitySprite s{};
    s.bodySrcX = 10;
    s.bodySrcY = 20;
    s.bodySrcW = 22;
    s.bodySrcH = 48;
    s.bodyStrideX = 24;
    s.bodyStrideY = 51;
    s.bodyCols = 5;
    FrameRect r = bodyFrameRectFor(s, Movement::DOWN, 99);
    EXPECT_EQ(r.x, 106);
}

TEST(CharacterSpritesTest, NewMonstersUseCustomGrid) {
    EXPECT_TRUE(spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::GOBLIN), 0)
                        .customGrid);
    EXPECT_TRUE(
            spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::ORC), 0).customGrid);
    EXPECT_TRUE(spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::GOLEM), 0)
                        .customGrid);
    EXPECT_TRUE(spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::SKELETON), 0)
                        .customGrid);
    EXPECT_TRUE(spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::SPIDER), 0)
                        .customGrid);
    EXPECT_TRUE(spriteForEntity(EntityType::MONSTER, static_cast<uint8_t>(NPCType::ZOMBIE), 0)
                        .customGrid);
}
