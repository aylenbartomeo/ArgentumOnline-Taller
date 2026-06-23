#include <gtest/gtest.h>

#include "GoldPrompt.h"

TEST(GoldPromptTest, EmptyDefaultsToOne) { EXPECT_EQ(goldAmountFromText(""), 1); }

TEST(GoldPromptTest, ParsesNumber) { EXPECT_EQ(goldAmountFromText("1000"), 1000); }

TEST(GoldPromptTest, ZeroBecomesOne) { EXPECT_EQ(goldAmountFromText("0"), 1); }

TEST(GoldPromptTest, ClampsToMaxUint16) {
    EXPECT_EQ(goldAmountFromText("65535"), 65535);
    EXPECT_EQ(goldAmountFromText("65536"), 65535);
    EXPECT_EQ(goldAmountFromText("99999"), 65535);
    EXPECT_EQ(goldAmountFromText("9999999"), 65535);
}
