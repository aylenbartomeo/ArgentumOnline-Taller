#ifndef CREATE_CHARACTER_DTO_H
#define CREATE_CHARACTER_DTO_H

#include <cstdint>

struct CreateCharacterDTO {
    uint8_t race = 0;
    uint8_t characterClass = 0;

    CreateCharacterDTO() = default;

    CreateCharacterDTO(uint8_t race, uint8_t characterClass):
            race(race), characterClass(characterClass) {}
};

#endif  // CREATE_CHARACTER_DTO_H
