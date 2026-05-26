#include <gtest/gtest.h>

#include "Camera.h"

TEST(CameraTest, ScreenToCellNoOffset) {
    Camera camera(640, 480, 32, 20, 15);
    Position cell = camera.screenToCell(40, 40);
    EXPECT_EQ(cell.x, 1);
    EXPECT_EQ(cell.y, 1);
}

TEST(CameraTest, CellToScreenNoOffset) {
    Camera camera(640, 480, 32, 20, 15);
    Position screen = camera.cellToScreen(2, 3);
    EXPECT_EQ(screen.x, 64);
    EXPECT_EQ(screen.y, 96);
}

TEST(CameraTest, MoveShiftsScreenPosition) {
    Camera camera(640, 480, 32, 100, 100);
    camera.move(32, 0);
    Position screen = camera.cellToScreen(2, 0);
    EXPECT_EQ(screen.x, 32);
}

TEST(CameraTest, MoveClampsAtZero) {
    Camera camera(640, 480, 32, 100, 100);
    camera.move(-100, -100);
    EXPECT_EQ(camera.getOffsetX(), 0);
    EXPECT_EQ(camera.getOffsetY(), 0);
}

TEST(CameraTest, MoveClampsAtMax) {
    Camera camera(640, 480, 32, 100, 100);
    camera.move(100000, 100000);
    EXPECT_EQ(camera.getOffsetX(), 100 * 32 - 640);
    EXPECT_EQ(camera.getOffsetY(), 100 * 32 - 480);
}

TEST(CameraTest, SmallMapDoesNotScroll) {
    Camera camera(640, 480, 32, 5, 5);
    camera.move(1000, 1000);
    EXPECT_EQ(camera.getOffsetX(), 0);
    EXPECT_EQ(camera.getOffsetY(), 0);
}
