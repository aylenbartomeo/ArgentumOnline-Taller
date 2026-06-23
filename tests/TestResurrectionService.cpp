#include <gtest/gtest.h>

#include "model/systems/ResurrectionService.h"

TEST(ResurrectionServiceTest, RequestFailsIfNotDead) {
    ResurrectionService svc;
    auto result = svc.requestResurrection(1, Position{10, 10}, false, {Position{15, 15}});
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.message, "No eres un fantasma.");
}

TEST(ResurrectionServiceTest, RequestFailsIfNoPriests) {
    ResurrectionService svc;
    auto result = svc.requestResurrection(1, Position{10, 10}, true, {});
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.message, "No hay sacerdotes en este mundo para resucitarte.");
}

TEST(ResurrectionServiceTest, RequestSuccessEnqueuesWithDelay) {
    ResurrectionService svc;
    std::vector<Position> priests = {Position{20, 10}, Position{12, 10}};

    auto result = svc.requestResurrection(1, Position{10, 10}, true, priests);
    EXPECT_TRUE(result.success);

    // Delay = 2 * 200 = 400ms. 400/1000 = 0 seconds string.
    EXPECT_EQ(result.message, "Resucitando... Por favor espera 0 segundos.");

    // Tick 200ms -> should not complete
    auto completed1 = svc.tick(200.0f);
    EXPECT_TRUE(completed1.empty());

    // Tick another 200ms -> should complete
    auto completed2 = svc.tick(200.0f);
    EXPECT_EQ(completed2.size(), 1u);
    EXPECT_EQ(completed2[0].playerDbId, 1u);
    EXPECT_EQ(completed2[0].targetPos.x, 12);
    EXPECT_EQ(completed2[0].targetPos.y, 10);
}
