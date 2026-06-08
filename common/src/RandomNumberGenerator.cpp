#include "RandomNumberGenerator.h"

#include <random>
#include <utility>

static std::mt19937& get_engine() {
    thread_local std::random_device rd;
    thread_local std::mt19937 generator(rd());
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
