#include "ManualPanel.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

ManualPanel::ManualPanel(const std::string& fontPath) : font(nullptr) {
    if (TTF_WasInit() == 0 && TTF_Init() != 0) {
        throw std::runtime_error(std::string("TTF_Init failed: ") + TTF_GetError());
    }
    font = TTF_OpenFont(fontPath.c_str(), 14); // 14px size
    if (!font) {
        throw std::runtime_error(std::string("Cannot open font: ") + TTF_GetError());
    }
}

ManualPanel::~ManualPanel() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
}

void ManualPanel::loadManual(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        lines.push_back("Error: No se pudo cargar " + filePath);
        return;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    parseMarkdown(buffer.str());
}

void ManualPanel::parseMarkdown(const std::string& content) {
    lines.clear();
    std::istringstream stream(content);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
}

void ManualPanel::update(const FrameInput& input, int windowW, int windowH) {
    if (!visible) return;

    // Cerrar si se pide salir
    if (input.quit) {
        visible = false;
        return;
    }

    const int panelX = (windowW - panelW) / 2;
    const int panelY = (windowH - panelH) / 2;

    bool mouseOverPanel = (input.mouseX >= panelX && input.mouseX <= panelX + panelW &&
                           input.mouseY >= panelY && input.mouseY <= panelY + panelH);

    // Cerrar al clickear afuera
    if (input.mouseLeftJustPressed && !mouseOverPanel) {
        visible = false;
        return;
    }

    // Scroll
    if (input.mouseScroll != 0 && mouseOverPanel) {
        scrollOffset -= input.mouseScroll * 30;
        if (scrollOffset < 0) {
            scrollOffset = 0;
        }
    }
}

void ManualPanel::render(SDL_Renderer* renderer, int windowW, int windowH) {
    if (!visible) return;

    const int panelX = (windowW - panelW) / 2;
    const int panelY = (windowH - panelH) / 2;

    // Fondo semi-transparente oscuro
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 230);
    SDL_Rect bg = {panelX, panelY, panelW, panelH};
    SDL_RenderFillRect(renderer, &bg);
    
    // Borde
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_RenderDrawRect(renderer, &bg);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Renderizar texto con recorte
    SDL_Rect oldClip;
    SDL_RenderGetClipRect(renderer, &oldClip);
    SDL_Rect textClip = {panelX + PADDING, panelY + PADDING, panelW - PADDING * 2, panelH - PADDING * 2};
    SDL_RenderSetClipRect(renderer, &textClip);

    const int textAreaW = textClip.w - 10; // menos espacio para el scrollbar
    int currentY = panelY + PADDING - scrollOffset;

    SDL_Color textColor = {220, 220, 220, 255};
    int renderedHeight = 0;

    for (const std::string& line : lines) {
        std::string textToRender = line;
        if (textToRender.empty()) textToRender = " ";

        SDL_Surface* surf = TTF_RenderUTF8_Blended_Wrapped(font, textToRender.c_str(), textColor, textAreaW);
        if (surf) {
            int h = surf->h;
            // Solo renderizar si está dentro de la vista
            if (currentY + h > textClip.y && currentY < textClip.y + textClip.h) {
                SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                if (tex) {
                    SDL_Rect dst = {panelX + PADDING, currentY, surf->w, h};
                    SDL_RenderCopy(renderer, tex, nullptr, &dst);
                    SDL_DestroyTexture(tex);
                }
            }
            currentY += h + 2;
            renderedHeight += h + 2;
            SDL_FreeSurface(surf);
        }
    }

    // Scrollbar lógica
    int maxScroll = renderedHeight - textClip.h + 20;
    if (maxScroll < 0) maxScroll = 0;
    
    if (scrollOffset > maxScroll) scrollOffset = maxScroll;

    if (maxScroll > 0) {
        int trackX = panelX + panelW - 10;
        int trackY = panelY + PADDING;
        int trackH = textClip.h;

        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_Rect track = {trackX, trackY, 6, trackH};
        SDL_RenderFillRect(renderer, &track);

        float visibleRatio = (float)textClip.h / renderedHeight;
        int thumbH = std::max(20, (int)(trackH * visibleRatio));

        float scrollPct = (float)scrollOffset / maxScroll;
        int thumbY = trackY + (int)((trackH - thumbH) * scrollPct);

        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        SDL_Rect thumb = {trackX, thumbY, 6, thumbH};
        SDL_RenderFillRect(renderer, &thumb);
    }

    // Restaurar clip
    if (SDL_RectEmpty(&oldClip)) {
        SDL_RenderSetClipRect(renderer, nullptr);
    } else {
        SDL_RenderSetClipRect(renderer, &oldClip);
    }
}
