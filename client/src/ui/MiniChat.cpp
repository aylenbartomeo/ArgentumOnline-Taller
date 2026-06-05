#include "MiniChat.h"

#include <algorithm>
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
    while (static_cast<int>(history.size()) > MAX_HISTORY) {
        history.pop_front();
    }
}

void MiniChat::update(const FrameInput& input, int windowH) {
    if (input.toggleChat) {
        visible = !visible;
    }

    // Forzamos visibilidad si se empieza a escribir
    if (input.chatInputActive) {
        visible = true;
    }

    if (!visible) {
        isResizing = false;
        return;
    }

    const int panelX = PADDING;
    const int panelY = windowH - panelH - PADDING;

    // SCROLL
    if (input.mouseScroll != 0 && isMouseOver(input.mouseX, input.mouseY, windowH)) {
        scrollOffset += input.mouseScroll * 2;  // Multiplicador de velocidad
        if (scrollOffset < 0)
            scrollOffset = 0;

        int maxVisibleLines = (panelH - LINE_H - PADDING * 3) / LINE_H;
        int maxScroll = std::max(0, static_cast<int>(history.size()) - maxVisibleLines);
        if (scrollOffset > maxScroll)
            scrollOffset = maxScroll;
    }

    // RESIZE (esquina superior derecha)
    const int handleSize = 15;
    SDL_Rect resizeHandle = {panelX + panelW - handleSize, panelY, handleSize, handleSize};
    bool mouseOnHandle =
            (input.mouseX >= resizeHandle.x && input.mouseX <= resizeHandle.x + resizeHandle.w &&
             input.mouseY >= resizeHandle.y && input.mouseY <= resizeHandle.y + resizeHandle.h);

    if (mouseOnHandle && input.mouseLeftJustPressed) {
        isResizing = true;
    }
    if (!input.mouseLeftHeld) {
        isResizing = false;
    }

    if (isResizing) {
        panelW = input.mouseX - panelX;
        panelH = windowH - input.mouseY - PADDING;

        // Limites para que no se rompa la UI
        if (panelW < 200)
            panelW = 200;
        if (panelW > 800)
            panelW = 800;
        if (panelH < 80)
            panelH = 80;
        if (panelH > 600)
            panelH = 600;

        // Ajustar el scroll por si la ventana se hizo más grande y muestra todo
        int maxVisibleLines = (panelH - LINE_H - PADDING * 3) / LINE_H;
        int maxScroll = std::max(0, static_cast<int>(history.size()) - maxVisibleLines);
        if (scrollOffset > maxScroll)
            scrollOffset = maxScroll;
    }
}

bool MiniChat::isMouseOver(int mx, int my, int windowH) const {
    if (!visible)
        return false;
    int px = PADDING;
    int py = windowH - panelH - PADDING;
    return mx >= px && mx <= px + panelW && my >= py && my <= py + panelH;
}

void MiniChat::renderLine(SDL_Renderer* renderer, const std::string& text, int x, int y, int maxW,
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
    if (w > maxW)
        w = maxW;  // Ajusta visualmente el ancho de texto

    SDL_Rect dst = {x, y, w, h};
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}

void MiniChat::render(SDL_Renderer* renderer, int windowW, int windowH, bool inputActive,
                      const std::string& inputText) {
    (void)windowW;
    const int panelX = PADDING;
    const int panelY = windowH - panelH - PADDING;

    // Fondo semi-transparente
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 140);
    SDL_Rect bg = {panelX, panelY, panelW, panelH};
    SDL_RenderFillRect(renderer, &bg);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Botón / Handle para Resize
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_Rect handle = {panelX + panelW - 10, panelY + 2, 8, 8};
    SDL_RenderFillRect(renderer, &handle);

    // Calcular renderizado
    int sepY = panelY + panelH - LINE_H - PADDING * 2;
    int maxVisibleLines = (panelH - LINE_H - PADDING * 3) / LINE_H;
    int totalLines = static_cast<int>(history.size());

    int startIdx = totalLines - maxVisibleLines - scrollOffset;
    if (startIdx < 0)
        startIdx = 0;
    int endIdx = startIdx + maxVisibleLines;
    if (endIdx > totalLines)
        endIdx = totalLines;

    // Dibujar historial
    int lineY = panelY + PADDING;
    int textAreaW = panelW - PADDING * 2;
    for (int i = startIdx; i < endIdx; ++i) {
        const std::string& line = history[i];
        SDL_Color col = {255, 255, 255, 255};
        if (line.rfind("[Daño]", 0) == 0)
            col = {255, 220, 50, 255};
        if (line.rfind("[Recv]", 0) == 0)
            col = {255, 100, 100, 255};
        if (line.rfind("[PM]", 0) == 0)
            col = {80, 220, 255, 255};
        if (line.rfind("[Clan]", 0) == 0)
            col = {150, 255, 150, 255};
        renderLine(renderer, line, panelX + PADDING, lineY, textAreaW, col);
        lineY += LINE_H;
    }

    // Separador
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawLine(renderer, panelX, sepY, panelX + panelW, sepY);

    // Input del jugador
    std::string inputLine = inputActive ? ("> " + inputText + "_") : "> [Enter/F2]";
    const SDL_Color inputColor =
            inputActive ? SDL_Color{255, 220, 50, 255} : SDL_Color{160, 160, 160, 255};
    renderLine(renderer, inputLine, panelX + PADDING, panelY + panelH - LINE_H - PADDING, textAreaW,
               inputColor);
}
