#ifndef RANDOM_NUMBER_GENERATOR_H_
#define RANDOM_NUMBER_GENERATOR_H_

class RandomNumberGenerator {
public:
    RandomNumberGenerator() = default;
    ~RandomNumberGenerator() = default;

    // Genera un numero flotante aleatorio con distribucion uniforme en el rango [min, max).
    float operator()(float min, float max) const;

    // Genera un numero entero aleatorio con distribucion uniforme en el rango [min, max].
    int operator()(int min, int max) const;

    // Fija una semilla determinista (util para testing)
    static void setFixedSeed(unsigned int seed);
};

#endif  // RANDOM_NUMBER_GENERATOR_H_
