#ifndef JOIN_RESPONSE_DTO_H
#define JOIN_RESPONSE_DTO_H

#include <cstdint>
#include <vector>

struct JoinResponseDTO {
    bool needsCreation;
    
    // Only used if needsCreation is true
    // Base stats
    int baseStrength;
    int baseAgility;
    int baseIntelligence;
    int baseConstitution;

    // Race factors (4 races x 5 factors: life, mana, strength, agility, intelligence)
    std::vector<float> raceFactors;

    // Class factors (4 classes x 2 factors: life, mana)
    std::vector<float> classFactors;

    JoinResponseDTO() : needsCreation(false), baseStrength(0), baseAgility(0), baseIntelligence(0), baseConstitution(0) {}
};

#endif  // JOIN_RESPONSE_DTO_H
