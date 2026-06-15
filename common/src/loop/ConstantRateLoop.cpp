#include "loop/ConstantRateLoop.h"

#include <chrono>
#include <thread>

ConstantRateLoop::ConstantRateLoop(int64_t rateMs): rateMs(rateMs) {}

LoopStep ConstantRateLoop::nextStep(int64_t elapsedMs, int64_t rateMs) {
    int64_t rest = rateMs - elapsedMs;
    int64_t t1Delta = rateMs;
    int64_t iterationDelta = 1;

    if (rest < 0) {
        int64_t behind = -rest;
        rest = rateMs - behind % rateMs;
        int64_t lost = behind + rest;
        t1Delta += lost;
        iterationDelta += lost / rateMs;
    }

    return {rest, t1Delta, iterationDelta};
}

void ConstantRateLoop::run(const std::function<bool(int64_t)>& body) const {
    using clock = std::chrono::steady_clock;

    clock::time_point t1 = clock::now();
    int64_t iteration = 0;

    while (body(iteration)) {
        clock::time_point t2 = clock::now();
        int64_t elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

        LoopStep step = nextStep(elapsedMs, rateMs);
        if (step.sleepMs > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(step.sleepMs));
        }

        t1 += std::chrono::milliseconds(step.t1DeltaMs);
        iteration += step.iterationDelta;
    }
}
