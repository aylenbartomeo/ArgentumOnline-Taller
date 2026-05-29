#include <gtest/gtest.h>

#include "server/src/model/map/GroundItemLayer.h"

TEST(GroundItemLayerTest, PlaceItem_and_inspect) {
    GroundItemLayer layer;
    Position pos{10, 10};

    EXPECT_TRUE(layer.placeItem(pos, 1, 5));

    auto inspected = layer.inspectItem(pos);
    ASSERT_TRUE(inspected.has_value());
    EXPECT_EQ(inspected->itemId, 1);
    EXPECT_EQ(inspected->amount, 5);
}

TEST(GroundItemLayerTest, PickUpItem_removes_from_layer) {
    GroundItemLayer layer;
    Position pos{5, 5};
    layer.placeItem(pos, 2, 10);

    auto picked = layer.pickUpItem(pos);
    ASSERT_TRUE(picked.has_value());
    EXPECT_EQ(picked->itemId, 2);
    EXPECT_EQ(picked->amount, 10);

    EXPECT_FALSE(layer.hasItemAt(pos));
    EXPECT_FALSE(layer.inspectItem(pos).has_value());
}

TEST(GroundItemLayerTest, PickUpItem_empty_returns_nullopt) {
    GroundItemLayer layer;
    auto picked = layer.pickUpItem(Position{0, 0});
    EXPECT_FALSE(picked.has_value());
}

TEST(GroundItemLayerTest, PlaceItem_overwrites_existing) {
    GroundItemLayer layer;
    Position pos{2, 2};
    layer.placeItem(pos, 1, 10);
    layer.placeItem(pos, 2, 20);  // Overwrite

    auto inspected = layer.inspectItem(pos);
    ASSERT_TRUE(inspected.has_value());
    EXPECT_EQ(inspected->itemId, 2);
    EXPECT_EQ(inspected->amount, 20);
}

TEST(GroundItemLayerTest, HasItemAt_true_and_false) {
    GroundItemLayer layer;
    Position pos1{1, 1};
    Position pos2{2, 2};

    layer.placeItem(pos1, 1, 1);

    EXPECT_TRUE(layer.hasItemAt(pos1));
    EXPECT_FALSE(layer.hasItemAt(pos2));
}
