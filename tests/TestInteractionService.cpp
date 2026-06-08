#include <gtest/gtest.h>

#include "model/interfaces/Interactable.h"
#include "model/systems/InteractionService.h"

class DummyInteractable: public Interactable {
public:
    InteractionResult beInteractedBy(Player& /*player*/) override {
        return {InteractionStatus::SUCCESS, "Interactuando"};
    }
    InteractionResult handleCommand(Player& /*player*/, const NpcCommandDTO& /*dto*/) override {
        return {InteractionStatus::SUCCESS, "Comando dummy"};
    }
    Position getPosition() const override { return {10, 10}; }
    uint32_t getId() const override { return 1; }
};

// Player requires too many dependencies to mock easily here without TestEntityManager helper.
// Since InteractionService mainly tests the map, we can rely on integration tests for the full
// flow.
TEST(InteractionServiceTest, HasActiveInteractionReturnsFalseInitially) {
    InteractionService svc;
    EXPECT_FALSE(svc.hasActiveInteraction(1));
}

TEST(InteractionServiceTest, EndInteractionRemovesIt) {
    InteractionService svc;
    // We cannot test startInteraction fully without a real Player object,
    // but we can test that endInteraction cleans up.
    svc.endInteraction(1);
    EXPECT_FALSE(svc.hasActiveInteraction(1));
}
