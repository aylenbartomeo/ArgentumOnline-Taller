#include "CharacterCreationScreen.h"

#include <stdexcept>

#include <SDL2/SDL.h>

#include "loop/ConstantRateLoop.h"

namespace {
constexpr int MENU_FRAME_DURATION_MS = 33;
constexpr const char* FONT_PATH = "resources/DejaVuSans-Bold.ttf";
constexpr const char* BACKGROUND = "resources/CREAR-PERSONAJE.png";

const std::vector<std::string> RACE_NAMES = {"Humano", "Elfo", "Enano", "Gnomo"};
const std::vector<std::string> CLASS_NAMES = {"Mago", "Guerrero", "Paladín", "Clérigo"};

// Todas las coordenadas están en el espacio original de la imagen 800x600.
// Botones en espacio 800x600
constexpr int BTN_VOLVER_X = 50;
constexpr int BTN_VOLVER_Y = 520;
constexpr int BTN_VOLVER_W = 250;
constexpr int BTN_VOLVER_H = 60;

constexpr int BTN_CREAR_X = 500;
constexpr int BTN_CREAR_Y = 520;
constexpr int BTN_CREAR_W = 250;
constexpr int BTN_CREAR_H = 60;

const SDL_Color COLOR_WHITE = {255, 255, 255, 255};
const SDL_Color COLOR_GREEN = {50, 200, 50, 255};
const SDL_Color COLOR_RED = {200, 50, 50, 255};
}  // namespace

CharacterCreationScreen::CharacterCreationScreen(SDL2pp::Renderer& renderer,
                                                 TextureManager& textures,
                                                 const JoinResponseDTO& configs):
        renderer(renderer), textures(textures), configs(configs) {
    if (TTF_WasInit() == 0 && TTF_Init() != 0) {
        throw std::runtime_error(std::string("TTF_Init: ") + TTF_GetError());
    }
    font = TTF_OpenFont(FONT_PATH, 16);
    fontLarge = TTF_OpenFont(FONT_PATH, 24);
    if (!font || !fontLarge) {
        throw std::runtime_error(std::string("CharacterCreationScreen font: ") + TTF_GetError());
    }

    updateStats();
}

CharacterCreationScreen::~CharacterCreationScreen() {
    if (font)
        TTF_CloseFont(font);
    if (fontLarge)
        TTF_CloseFont(fontLarge);
}

void CharacterCreationScreen::updateStats() {
    int raceOffset = selectedRace * 5;
    float rLife = configs.raceFactors[raceOffset];
    float rMana = configs.raceFactors[raceOffset + 1];
    float rStr = configs.raceFactors[raceOffset + 2];
    float rAgi = configs.raceFactors[raceOffset + 3];
    float rInt = configs.raceFactors[raceOffset + 4];

    int classOffset = selectedClass * 2;
    float cLife = configs.classFactors[classOffset];
    float cMana = configs.classFactors[classOffset + 1];

    currentHp = static_cast<int>(configs.baseConstitution * rLife * cLife);
    currentMana = static_cast<int>(configs.baseIntelligence * rMana * cMana);
    currentStrength = static_cast<int>(configs.baseStrength * rStr);
    currentAgility = static_cast<int>(configs.baseAgility * rAgi);
    currentIntelligence = static_cast<int>(configs.baseIntelligence * rInt);
}

void CharacterCreationScreen::drawText(const std::string& text, int x, int y, TTF_Font* useFont,
                                       const SDL_Color& color) {
    SDL_Surface* surf = TTF_RenderUTF8_Blended(useFont, text.c_str(), color);
    if (!surf)
        return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer.Get(), surf);
    SDL_Rect dst{x, y, surf->w, surf->h};
    SDL_FreeSurface(surf);
    if (!tex)
        return;
    SDL_RenderCopy(renderer.Get(), tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
}

void CharacterCreationScreen::drawStatRow(const std::string& label, int value, float raceFactor,
                                          int xLeft, int xRight, int y) {
    // Label a la izquierda
    drawText(label, xLeft, y, font, COLOR_WHITE);

    // Número a la derecha, con color según el factor
    SDL_Color color = COLOR_WHITE;
    if (raceFactor > 1.0f)
        color = COLOR_GREEN;
    else if (raceFactor < 1.0f)
        color = COLOR_RED;

    std::string numStr = std::to_string(value);

    // Calcular ancho para alinear a la derecha
    int w = 0, h = 0;
    if (font)
        TTF_SizeUTF8(font, numStr.c_str(), &w, &h);

    drawText(numStr, xRight - w, y, font, color);
}

void CharacterCreationScreen::drawSelector(const std::string& title, const std::string& value,
                                           int y) {
    int centerX = 585 + 112;

    // Título centrado
    int tw = 0, th = 0;
    if (fontLarge)
        TTF_SizeUTF8(fontLarge, title.c_str(), &tw, &th);
    drawText(title, centerX - tw / 2, y, fontLarge, COLOR_WHITE);

    // Flechas fijas
    drawText("<", centerX - 100, y + 40, font, COLOR_WHITE);
    drawText(">", centerX + 80, y + 40, font, COLOR_WHITE);

    // Valor centrado entre las flechas
    int vw = 0, vh = 0;
    if (font)
        TTF_SizeUTF8(font, value.c_str(), &vw, &vh);
    drawText(value, centerX - vw / 2, y + 40, font, COLOR_WHITE);
}

void CharacterCreationScreen::drawPreview(int x, int y) {
    std::string folder;
    switch (selectedRace) {
        case 0:
            folder = "resources/humano/";
            break;
        case 1:
            folder = "resources/elfo/";
            break;
        case 2:
            folder = "resources/enano/";
            break;
        case 3:
            folder = "resources/gnomo/";
            break;
    }

    try {
        SDL2pp::Texture& headTex =
                textures.get(folder + "cabeza-" + folder.substr(10, folder.length() - 11) + ".png");
        SDL2pp::Texture& bodyTex =
                textures.get(folder + "cuerpo-" + folder.substr(10, folder.length() - 11) + ".png");

        int bw = 25;
        int bh = 45;
        int hw = 17;
        int hh = 17;
        int scale = 3;

        renderer.Copy(bodyTex, SDL2pp::Rect(0, 0, bw, bh),
                      SDL2pp::Rect(x, y, bw * scale, bh * scale));

        int headX = x + (bw * scale - hw * scale) / 2;

        // El humano y el elfo quedan perfectos con y - 9 * scale
        int headY = y - (9 * scale);

        // El enano y el gnomo necesitan que la cabeza esté un poco más abajo
        if (selectedRace == 2 || selectedRace == 3) {
            headY += 8 * scale;
        }

        renderer.Copy(headTex, SDL2pp::Rect(0, 0, hw, hh),
                      SDL2pp::Rect(headX, headY, hw * scale, hh * scale));
    } catch (...) {}
}

void CharacterCreationScreen::render() {
    renderer.Clear();

    int offX = 112;
    int offY = 84;

    // Dibujamos la imagen original de 800x600 centrada
    try {
        SDL2pp::Texture& bg = textures.get(BACKGROUND);
        renderer.Copy(bg, SDL2pp::NullOpt, SDL2pp::Rect(offX, offY, 800, 600));
    } catch (const std::exception& e) {
        std::cerr << "Error cargando fondo: " << e.what() << std::endl;
    }

    // Coordenadas ajustadas a 800x600 y centradas
    drawSelector("Clase", CLASS_NAMES[selectedClass], 75 + offY);
    drawSelector("Raza", RACE_NAMES[selectedRace], 165 + offY);

    // Atributos
    int attrY = 285 + offY;
    int xLeft = 470 + offX;
    int xRight = 700 + offX;
    int spacing = 30;

    int tw = 0, th = 0;
    if (fontLarge)
        TTF_SizeUTF8(fontLarge, "Atributos", &tw, &th);
    drawText("Atributos", 585 + offX - tw / 2, attrY, fontLarge, COLOR_WHITE);

    attrY += 50;

    int raceOffset = selectedRace * 5;
    drawStatRow("Fuerza", currentStrength, configs.raceFactors[raceOffset + 2], xLeft, xRight,
                attrY);
    attrY += spacing;
    drawStatRow("Agilidad", currentAgility, configs.raceFactors[raceOffset + 3], xLeft, xRight,
                attrY);
    attrY += spacing;
    drawStatRow("Inteligencia", currentIntelligence, configs.raceFactors[raceOffset + 4], xLeft,
                xRight, attrY);
    attrY += spacing;

    drawStatRow("Vida Máxima", currentHp, configs.raceFactors[raceOffset], xLeft, xRight, attrY);
    attrY += spacing;

    drawStatRow("Maná Máxima", currentMana, configs.raceFactors[raceOffset + 1], xLeft, xRight,
                attrY);

    // Preview
    tw = 0;
    th = 0;
    if (fontLarge)
        TTF_SizeUTF8(fontLarge, "Aspecto", &tw, &th);
    drawText("Aspecto", 235 + offX - tw / 2, 150 + offY, fontLarge, COLOR_WHITE);
    drawPreview(195 + offX, 240 + offY);

    renderer.Present();
}

CharacterCreationScreen::CreationResult CharacterCreationScreen::run() {
    bool running = true;
    CreationResult result = {false, 0, 0};

    ConstantRateLoop loop(MENU_FRAME_DURATION_MS);
    loop.run([&](int64_t) -> bool {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    // Compensamos el offset visual
                    int x = event.button.x - 112;
                    int y = event.button.y - 84;

                    // Las flechas ahora están en posiciones fijas (X=485 y X=665 aprox)
                    // Flechas Raza (Y=165 + 40 = 205)
                    if (y >= 185 && y <= 230) {
                        if (x >= 470 && x <= 510) {
                            selectedRace = (selectedRace - 1 + 4) % 4;
                            updateStats();
                        } else if (x >= 650 && x <= 690) {
                            selectedRace = (selectedRace + 1) % 4;
                            updateStats();
                        }
                    }

                    // Flechas Clase (Y=75 + 35 = 110)
                    if (y >= 90 && y <= 135) {
                        if (x >= 470 && x <= 510) {
                            selectedClass = (selectedClass - 1 + 4) % 4;
                            updateStats();
                        } else if (x >= 650 && x <= 690) {
                            selectedClass = (selectedClass + 1) % 4;
                            updateStats();
                        }
                    }

                    // Botón VOLVER
                    if (y >= BTN_VOLVER_Y && y <= BTN_VOLVER_Y + BTN_VOLVER_H &&
                        x >= BTN_VOLVER_X && x <= BTN_VOLVER_X + BTN_VOLVER_W) {
                        result.created = false;
                        running = false;
                    }

                    // Botón CREAR PERSONAJE
                    if (y >= BTN_CREAR_Y && y <= BTN_CREAR_Y + BTN_CREAR_H && x >= BTN_CREAR_X &&
                        x <= BTN_CREAR_X + BTN_CREAR_W) {
                        result.created = true;
                        result.race = static_cast<uint8_t>(selectedRace);
                        result.characterClass = static_cast<uint8_t>(selectedClass);
                        running = false;
                    }
                }
            }
        }
        render();
        return running;
    });

    return result;
}
