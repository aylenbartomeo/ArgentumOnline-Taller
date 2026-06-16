#include <vector>

#include <gtest/gtest.h>

#include "loop/ConstantRateLoop.h"

TEST(ConstantRateLoopTest, HappyPathSleepsTheRemainingTime) {
    LoopStep step = ConstantRateLoop::nextStep(10, 33);
    EXPECT_EQ(step.sleepMs, 23);
    EXPECT_EQ(step.t1DeltaMs, 33);
    EXPECT_EQ(step.iterationDelta, 1);
}

TEST(ConstantRateLoopTest, ExactlyOnTimeDoesNotSleep) {
    LoopStep step = ConstantRateLoop::nextStep(33, 33);
    EXPECT_EQ(step.sleepMs, 0);
    EXPECT_EQ(step.t1DeltaMs, 33);
    EXPECT_EQ(step.iterationDelta, 1);
}

TEST(ConstantRateLoopTest, BehindWithinOneFrameDropsThatIteration) {
    LoopStep step = ConstantRateLoop::nextStep(40, 33);
    EXPECT_EQ(step.sleepMs, 26);
    EXPECT_EQ(step.t1DeltaMs, 66);
    EXPECT_EQ(step.iterationDelta, 2);
}

TEST(ConstantRateLoopTest, BehindSeveralFramesSkipsProportionally) {
    LoopStep step = ConstantRateLoop::nextStep(70, 33);
    EXPECT_EQ(step.sleepMs, 29);
    EXPECT_EQ(step.t1DeltaMs, 99);
    EXPECT_EQ(step.iterationDelta, 3);
}

TEST(ConstantRateLoopTest, RunCallsBodyUntilItReturnsFalseAndAdvancesIteration) {
    ConstantRateLoop loop(1);
    int calls = 0;
    std::vector<int64_t> seenIterations;

    loop.run([&](int64_t iteration) {
        seenIterations.push_back(iteration);
        ++calls;
        return calls < 5;
    });

    EXPECT_EQ(calls, 5);
    ASSERT_EQ(seenIterations.size(), 5u);
    EXPECT_EQ(seenIterations.front(), 0);
    EXPECT_EQ(seenIterations.back(), 4);
}
