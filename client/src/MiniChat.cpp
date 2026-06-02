#include "MiniChat.h"

#include <stdexcept>

MiniChat::MiniChat(const std::string& fontPath): font(nullptr) {
    if (TTF_WasInit() == 0) {
        if (TTF_Init() != 0) {
            throw std::runtime_error(std::string("TTF_Init failed: ") + TTF_GetError());
        }
    }
    font = TTF_OpenFont(fontPath.c_str(), FONT_SIZE);
    if (!font) {
        throw std::runtime_error(std::string("Cannot open font: ") + TTF_GetError());
    }
}

MiniChat::~MiniChat() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
}

void MiniChat::pushMessage(const std::string& msg) {
    history.push_back(msg);
    while (static_cast<int>(history.size()) > MAX_LINES) {
        history.pop_front();
    }
}

void MiniChat::renderLine(SDL_Renderer* renderer, const std::string& text, int x, int y,
                          SDL_Color color) {
    if (text.empty())
        return;

    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface)
        return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture)
        return;

    int w, h;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
    if (w > PANEL_W - PADDING * 2)
        w = PANEL_W - PADDING * 2;

    SDL_Rect dst = {x, y, w, h};
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}

void MiniChat::render(SDL_Renderer* renderer, int windowW, int windowH, bool inputActive,
                      const std::string& inputText) {
    (void)windowW;
    const int panelX = PADDING;
    const int panelY = windowH - PANEL_H - PADDING;

    // Fondo semi-transparente
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 140);
    SDL_Rect bg = {panelX, panelY, PANEL_W, PANEL_H};
    SDL_RenderFillRect(renderer, &bg);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Historial de mensajes
    const SDL_Color white = {255, 255, 255, 255};
    const SDL_Color yellow = {255, 220, 50, 255};
    const SDL_Color cyan = {80, 220, 255, 255};

    int lineY = panelY + PADDING;
    for (const std::string& line: history) {
        SDL_Color col = white;
        if (line.rfind("[Daño]", 0) == 0)
            col = yellow;
        if (line.rfind("[Recv]", 0) == 0)
            col = {255, 100, 100, 255};
        if (line.rfind("[PM]", 0) == 0)
            col = cyan;
        if (line.rfind("[Clan]", 0) == 0)
            col = {150, 255, 150, 255};
        renderLine(renderer, line, panelX + PADDING, lineY, col);
        lineY += LINE_H;
    }

    // Separador
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawLine(renderer, panelX, panelY + PANEL_H - LINE_H - PADDING * 2, panelX + PANEL_W,
                       panelY + PANEL_H - LINE_H - PADDING * 2);

    // Input del jugador
    std::string inputLine;
    if (inputActive) {
        inputLine = "> " + inputText + "_";
    } else {
        inputLine = "> [Enter para escribir]";
    }
    const SDL_Color inputColor = inputActive ? yellow : SDL_Color{160, 160, 160, 255};
    renderLine(renderer, inputLine, panelX + PADDING, panelY + PANEL_H - LINE_H - PADDING,
               inputColor);
}
