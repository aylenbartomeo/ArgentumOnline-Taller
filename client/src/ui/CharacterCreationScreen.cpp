#include "CharacterCreationScreen.h"

#include <stdexcept>

#include <SDL2/SDL.h>

#include "loop/ConstantRateLoop.h"

namespace {
constexpr int MENU_FRAME_DURATION_MS = 33;
constexpr const char* FONT_PATH = "resources/DejaVuSans-Bold.ttf";
constexpr const char* BACKGROUND = "resources/pantallas/CREAR-PERSONAJE.png";

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
const SDL_Color COLOR_GOLD = {228, 194, 159, 255};
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
    drawText(label, xLeft, y, font, COLOR_GOLD);

    // Número a la derecha, con color según el factor
    SDL_Color color = COLOR_GOLD;
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
                                           int x, int y) {
    int centerX = x;

    // Título centrado
    int tw = 0, th = 0;
    if (fontLarge)
        TTF_SizeUTF8(fontLarge, title.c_str(), &tw, &th);
    drawText(title, centerX - tw / 2, y, fontLarge, COLOR_GOLD);

    // Flechas fijas
    drawText("<", centerX - 70, y + 40, font, COLOR_GOLD);
    drawText(">", centerX + 60, y + 40, font, COLOR_GOLD);

    // Valor centrado entre las flechas
    int vw = 0, vh = 0;
    if (font)
        TTF_SizeUTF8(font, value.c_str(), &vw, &vh);
    drawText(value, centerX - vw / 2, y + 40, font, COLOR_GOLD);
}

void CharacterCreationScreen::drawPreview(int x, int y) {
    int srcHx, srcHy, srcHw, srcHh;
    switch (selectedRace) {
        case 0:
            srcHx = 0;
            srcHy = 0;
            srcHw = 16;
            srcHh = 16;
            break;  // Humano
        case 1:
            srcHx = 0;
            srcHy = 0;
            srcHw = 17;
            srcHh = 20;
            break;  // Elfo
        case 2:
            srcHx = 2;
            srcHy = 2;
            srcHw = 13;
            srcHh = 19;
            break;  // Enano
        case 3:
            srcHx = 2;
            srcHy = 2;
            srcHw = 10;
            srcHh = 11;
            break;  // Gnomo
        default:
            srcHx = 0;
            srcHy = 0;
            srcHw = 16;
            srcHh = 16;
            break;
    }

    std::string bodySheet;
    std::string headSheet;
    switch (selectedRace) {
        case 0:
            bodySheet = "resources/race/human/human-body.png";
            headSheet = "resources/race/human/human-head.png";
            break;
        case 1:
            bodySheet = "resources/race/elf/elf-body.png";
            headSheet = "resources/race/elf/elf-head.png";
            break;
        case 2:
            bodySheet = "resources/race/dwarf/dwarf-body.png";
            headSheet = "resources/race/dwarf/dwarf-head.png";
            break;
        case 3:
            bodySheet = "resources/race/gnome/gnome-body.png";
            headSheet = "resources/race/gnome/gnome-head.png";
            break;
    }

    try {
        SDL2pp::Texture& headTex = textures.get(headSheet);
        SDL2pp::Texture& bodyTex = textures.get(bodySheet);
        int scale = 3;
        int srcBx, srcBy, srcBw, srcBh;

        switch (selectedRace) {
            case 2:  // Enano
            case 3:  // Gnomo
                srcBx = 3;
                srcBy = 12;
                srcBw = 14;
                srcBh = 23;
                break;
            default:  // Humano y Elfo
                srcBx = 3;
                srcBy = 5;
                srcBw = 20;
                srcBh = 39;
                break;
        }

        // x lo vamos a usar como Centro
        int renderX = x - (srcBw * scale) / 2;

        // Alineamos los pies a la misma altura usando la altura del humano (39) como base
        int renderY = y + (39 - srcBh) * scale;

        // Dibujar Cuerpo
        renderer.Copy(bodyTex, SDL2pp::Rect(srcBx, srcBy, srcBw, srcBh),
                      SDL2pp::Rect(renderX, renderY, srcBw * scale, srcBh * scale));

        // Calcular posición de la cabeza
        int headX = renderX + (srcBw * scale - srcHw * scale) / 2;
        int headY;

        switch (selectedRace) {
            case 0:                              // Humano
                headX -= 1;                      // Un pixel a la izquierda
                headY = renderY - (13 * scale);  // Subimos más
                break;
            case 1:                              // Elfo
                headY = renderY - (12 * scale);  // Subimos más
                break;
            case 2:                              // Enano
                headY = renderY - (13 * scale);  // Lo subimos bastante más (antes 9)
                break;
            case 3:                             // Gnomo
                headX += 1;                     // Correr un píxel a la derecha
                headY = renderY - (8 * scale);  // Subir apenas la cabeza
                break;
            default:
                headY = renderY - (11 * scale);
                break;
        }

        // Dibujar Cabeza
        renderer.Copy(headTex, SDL2pp::Rect(srcHx, srcHy, srcHw, srcHh),
                      SDL2pp::Rect(headX, headY, srcHw * scale, srcHh * scale));
    } catch (const std::exception& e) {}
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
    int rightColX = 550 + offX;
    int leftColX = 280 + offX;

    int claseY = 165 + offY;
    int razaY = 240 + offY;
    int attrY = 325 + offY;
    int aspectoY = 195 + offY;
    int previewY = 295 + offY;

    drawSelector("Clase", CLASS_NAMES[selectedClass], rightColX, claseY);
    drawSelector("Raza", RACE_NAMES[selectedRace], rightColX, razaY);

    // Atributos
    int xLeft = rightColX - 80;
    int xRight = rightColX + 80;
    int spacing = 22;

    int tw = 0, th = 0;
    if (fontLarge)
        TTF_SizeUTF8(fontLarge, "Atributos", &tw, &th);
    drawText("Atributos", rightColX - tw / 2, attrY, fontLarge, COLOR_GOLD);

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

    drawText("Aspecto", leftColX - tw / 2, aspectoY, fontLarge, COLOR_GOLD);
    drawPreview(leftColX, previewY);  // Le pasamos el centro para que lo dibuje alineado

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

                    int rightColX = 550;
                    int claseY = 165;
                    int razaY = 240;

                    // Flechas Raza
                    if (y >= razaY + 20 && y <= razaY + 60) {
                        if (x >= rightColX - 85 && x <= rightColX - 45) {
                            selectedRace = (selectedRace - 1 + 4) % 4;
                            updateStats();
                        } else if (x >= rightColX + 45 && x <= rightColX + 85) {
                            selectedRace = (selectedRace + 1) % 4;
                            updateStats();
                        }
                    }

                    // Flechas Clase
                    if (y >= claseY + 20 && y <= claseY + 60) {
                        if (x >= rightColX - 85 && x <= rightColX - 45) {
                            selectedClass = (selectedClass - 1 + 4) % 4;
                            updateStats();
                        } else if (x >= rightColX + 45 && x <= rightColX + 85) {
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
