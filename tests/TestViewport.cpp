#include <gtest/gtest.h>

#include "../client/src/rendering/Viewport.h"

TEST(ViewportTest, CentersOnFocusWhenThereIsRoom) {
    CameraOffset offset = computeCameraOffset(1000, 1000, 640, 480, 1920, 1920);
    EXPECT_EQ(offset.x, 680);
    EXPECT_EQ(offset.y, 760);
}

TEST(ViewportTest, ClampsToTopLeftAtZero) {
    CameraOffset offset = computeCameraOffset(100, 80, 640, 480, 1920, 1920);
    EXPECT_EQ(offset.x, 0);
    EXPECT_EQ(offset.y, 0);
}

TEST(ViewportTest, ClampsToBottomRight) {
    CameraOffset offset = computeCameraOffset(1900, 1900, 640, 480, 1920, 1920);
    EXPECT_EQ(offset.x, 1280);
    EXPECT_EQ(offset.y, 1440);
}

TEST(ViewportTest, WorldSmallerThanViewportIsZero) {
    CameraOffset offset = computeCameraOffset(160, 160, 640, 480, 320, 320);
    EXPECT_EQ(offset.x, 0);
    EXPECT_EQ(offset.y, 0);
}

TEST(ViewportTest, ClampsOneAxisWhileCenteringTheOther) {
    CameraOffset offset = computeCameraOffset(100, 1000, 640, 480, 1920, 1920);
    EXPECT_EQ(offset.x, 0);
    EXPECT_EQ(offset.y, 760);
}
