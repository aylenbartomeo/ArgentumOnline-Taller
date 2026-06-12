#ifndef CHARACTER_CREATION_SCREEN_H
#define CHARACTER_CREATION_SCREEN_H

#include <string>
#include <vector>

#include <SDL2pp/SDL2pp.hh>
#include <SDL_ttf.h>

#include "common/include/dto/JoinResponseDTO.h"
#include "common/include/dto/CreateCharacterDTO.h"
#include "../rendering/TextureManager.h"

class CharacterCreationScreen {
public:
    struct CreationResult {
        bool created; // true = "CREAR PERSONAJE", false = "VOLVER"
        uint8_t race;
        uint8_t characterClass;
    };

    CharacterCreationScreen(SDL2pp::Renderer& renderer,
                            TextureManager& textures, const JoinResponseDTO& configs);
    ~CharacterCreationScreen();

    CreationResult run();

private:
    SDL2pp::Renderer& renderer;
    TextureManager& textures;
    const JoinResponseDTO& configs;

    TTF_Font* font;
    TTF_Font* fontLarge;

    int selectedRace = 0; // 0=Human, 1=Elf, 2=Dwarf, 3=Gnome
    int selectedClass = 0; // 0=Mage, 1=Warrior, 2=Paladin, 3=Cleric

    // Calculated stats
    int currentHp = 0;
    int currentMana = 0;
    int currentStrength = 0;
    int currentAgility = 0;
    int currentIntelligence = 0;

    void updateStats();
    void render();
    void drawText(const std::string& text, int x, int y, TTF_Font* useFont, const SDL_Color& color);
    void drawStatRow(const std::string& label, int value, float raceFactor, int xLeft, int xRight, int y);
    void drawSelector(const std::string& title, const std::string& value, int y);
    void drawPreview(int x, int y);
};

#endif
