#include <gtest/gtest.h>

#include "../client/src/animation/CharacterAnimator.h"

TEST(CharacterAnimatorTest, DirectionFromDelta) {
    EXPECT_EQ(directionFromDelta(1, 0), Movement::RIGHT);
    EXPECT_EQ(directionFromDelta(-1, 0), Movement::LEFT);
    EXPECT_EQ(directionFromDelta(0, 1), Movement::DOWN);
    EXPECT_EQ(directionFromDelta(0, -1), Movement::UP);
}

TEST(CharacterAnimatorTest, BodyFrameRect) {
    FrameRect a = bodyFrameRect(Movement::DOWN, 0);
    EXPECT_EQ(a.x, 2);
    EXPECT_EQ(a.y, 4);
    EXPECT_EQ(a.w, 24);
    EXPECT_EQ(a.h, 44);

    FrameRect b = bodyFrameRect(Movement::DOWN, 1);
    EXPECT_EQ(b.x, 27);
    EXPECT_EQ(b.y, 4);

    FrameRect c = bodyFrameRect(Movement::UP, 0);
    EXPECT_EQ(c.x, 2);
    EXPECT_EQ(c.y, 48);

    FrameRect d = bodyFrameRect(Movement::RIGHT, 5);
    EXPECT_EQ(d.x, 126);
    EXPECT_EQ(d.y, 137);
}

TEST(CharacterAnimatorTest, HeadFrameRect) {
    FrameRect a = headFrameRect(Movement::DOWN);
    EXPECT_EQ(a.x, 6);
    EXPECT_EQ(a.y, 13);
    EXPECT_EQ(a.w, 13);
    EXPECT_EQ(a.h, 15);

    FrameRect b = headFrameRect(Movement::UP);
    EXPECT_EQ(b.y, 77);
}

TEST(CharacterAnimatorTest, StartsIdleFacingDown) {
    CharacterAnimator anim;
    EXPECT_EQ(anim.getFacing(), Movement::DOWN);
    EXPECT_EQ(anim.frameColumn(0), 0);
}

TEST(CharacterAnimatorTest, StepSetsFacingAndWalks) {
    CharacterAnimator anim;
    anim.update(0, 0, 0);
    anim.update(1, 0, 100);
    EXPECT_EQ(anim.getFacing(), Movement::RIGHT);
    EXPECT_EQ(anim.frameColumn(150), 2);
}

TEST(CharacterAnimatorTest, GoesIdleAfterTimeout) {
    CharacterAnimator anim;
    anim.update(0, 0, 0);
    anim.update(1, 0, 100);
    EXPECT_EQ(anim.frameColumn(400), 0);
}

TEST(CharacterAnimatorTest, VirtualStartsAtFirstTarget) {
    CharacterAnimator anim;
    anim.update(5, 5, 0);
    EXPECT_FLOAT_EQ(anim.getVirtualX(), 5.0f);
    EXPECT_FLOAT_EQ(anim.getVirtualY(), 5.0f);
}

TEST(CharacterAnimatorTest, VirtualSlidesTowardTarget) {
    CharacterAnimator anim;
    anim.update(5, 5, 0);
    anim.update(6, 5, 100);
    EXPECT_GT(anim.getVirtualX(), 5.0f);
    EXPECT_LT(anim.getVirtualX(), 6.0f);
    EXPECT_FLOAT_EQ(anim.getVirtualY(), 5.0f);
}

TEST(CharacterAnimatorTest, VirtualSnapsOnFarJump) {
    CharacterAnimator anim;
    anim.update(0, 0, 0);
    anim.update(40, 40, 100);
    EXPECT_FLOAT_EQ(anim.getVirtualX(), 40.0f);
    EXPECT_FLOAT_EQ(anim.getVirtualY(), 40.0f);
}
