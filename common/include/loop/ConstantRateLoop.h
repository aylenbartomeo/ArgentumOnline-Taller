#ifndef CONSTANT_RATE_LOOP_H
#define CONSTANT_RATE_LOOP_H

#include <cstdint>
#include <functional>

struct LoopStep {
    int64_t sleepMs;
    int64_t t1DeltaMs;
    int64_t iterationDelta;
};

class ConstantRateLoop {
public:
    explicit ConstantRateLoop(int64_t rateMs);

    static LoopStep nextStep(int64_t elapsedMs, int64_t rateMs);

    void run(const std::function<bool(int64_t)>& body) const;

private:
    int64_t rateMs;
};

#endif
