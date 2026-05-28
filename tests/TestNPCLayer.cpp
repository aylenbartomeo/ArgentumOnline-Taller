#include <gtest/gtest.h>
#include "server/src/model/map/NPCLayer.h"

TEST(NPCLayerTest, AddNPC_and_findAt) {
    NPCLayer layer;
    Position pos{5, 5};
    
    uint32_t id = layer.addNPC(NPCType::PRIEST, pos);
    EXPECT_GT(id, 0);
    
    auto found = layer.findNPCAt(pos);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->type, NPCType::PRIEST);
    EXPECT_EQ(found->id, id);
    
    EXPECT_FALSE(layer.findNPCAt(Position{6, 6}).has_value());
}

TEST(NPCLayerTest, FindNPCsInRange_filters_correctly) {
    NPCLayer layer;
    layer.addNPC(NPCType::MERCHANT, Position{10, 10});
    layer.addNPC(NPCType::BANKER, Position{12, 12});
    layer.addNPC(NPCType::PRIEST, Position{20, 20}); // Far away
    
    auto inRange = layer.findNPCsInRange(Position{11, 11}, 2); // Distance calculated as Chebyshev max(|dx|, |dy|)
    // 10,10 is dist 1. 12,12 is dist 1. 20,20 is dist 9.
    
    EXPECT_EQ(inRange.size(), 2);
    
    bool foundMerchant = false;
    bool foundBanker = false;
    
    for (const auto& npc : inRange) {
        if (npc.type == NPCType::MERCHANT) foundMerchant = true;
        if (npc.type == NPCType::BANKER) foundBanker = true;
    }
    
    EXPECT_TRUE(foundMerchant);
    EXPECT_TRUE(foundBanker);
}
