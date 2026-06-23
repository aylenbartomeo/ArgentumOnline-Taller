#include "RandomNumberGenerator.h"

#include <optional>
#include <random>
#include <utility>

static std::optional<unsigned int> global_fixed_seed;

void RandomNumberGenerator::setFixedSeed(unsigned int seed) { global_fixed_seed = seed; }

static std::mt19937& get_engine() {
    thread_local bool initialized = false;
    thread_local std::mt19937 generator;

    if (!initialized) {
        if (global_fixed_seed.has_value()) {
            generator.seed(global_fixed_seed.value());
        } else {
            std::random_device rd;
            generator.seed(rd());
        }
        initialized = true;
    }
    return generator;
}

float RandomNumberGenerator::operator()(float min, float max) const {
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(get_engine());
}

int RandomNumberGenerator::operator()(int min, int max) const {
    if (min > max) {
        std::swap(min, max);
    }
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(get_engine());
}
