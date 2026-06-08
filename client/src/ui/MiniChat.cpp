#include "MiniChat.h"

#include <algorithm>
#include <stdexcept>
#include <vector>

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

void MiniChat::update(const FrameInput& input, int windowW, int windowH) {
    if (input.toggleChat) {
        visible = !visible;
    }

    if (input.chatInputActive && !visible) {
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
        scrollOffset += input.mouseScroll * 3;
        if (scrollOffset < 0)
            scrollOffset = 0;
    }

    const int handleSize = 16;
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

        if (panelW < 200)
            panelW = 200;
        if (panelH < 80)
            panelH = 80;

        int maxW = windowW - PADDING * 2;
        int maxH = windowH - PADDING * 2;

        if (panelW > maxW)
            panelW = maxW;
        if (panelH > maxH)
            panelH = maxH;
    }
}

bool MiniChat::isMouseOver(int mx, int my, int windowH) const {
    if (!visible)
        return false;
    int px = PADDING;
    int py = windowH - panelH - PADDING;
    return mx >= px && mx <= px + panelW && my >= py && my <= py + panelH;
}

void MiniChat::render(SDL_Renderer* renderer, int windowW, int windowH, bool inputActive,
                      const std::string& inputText) {
    (void)windowW;
    if (!visible)
        return;
    const int panelX = PADDING;
    const int panelY = windowH - panelH - PADDING;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect bg = {panelX, panelY, panelW, panelH};
    SDL_RenderFillRect(renderer, &bg);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    SDL_Rect oldClip;
    SDL_RenderGetClipRect(renderer, &oldClip);
    SDL_Rect chatClip = {panelX, panelY, panelW, panelH};
    SDL_RenderSetClipRect(renderer, &chatClip);

    int textAreaW = panelW - PADDING * 3 - 10;

    // Renderizar input
    std::string inputLine = inputActive ? ("> " + inputText + "_") : "> [Enter/F2]";
    const SDL_Color inputColor =
            inputActive ? SDL_Color{255, 220, 50, 255} : SDL_Color{160, 160, 160, 255};

    SDL_Surface* inpSurf =
            TTF_RenderUTF8_Blended_Wrapped(font, inputLine.c_str(), inputColor, textAreaW);
    int inpW = 0, inpH = 0;
    SDL_Texture* inpTex = nullptr;
    if (inpSurf) {
        inpW = inpSurf->w;
        inpH = inpSurf->h;
        inpTex = SDL_CreateTextureFromSurface(renderer, inpSurf);
        SDL_FreeSurface(inpSurf);
    }

    int bottomY = panelY + panelH - inpH - PADDING;

    // Renderizar historial
    int currentY = bottomY - PADDING;
    int totalMsgs = history.size();
    int renderedMsgs = 0;

    int startIdx = totalMsgs - 1 - scrollOffset;
    if (startIdx >= totalMsgs)
        startIdx = totalMsgs - 1;

    std::vector<SDL_Rect> destRects;
    std::vector<SDL_Texture*> textures;

    for (int i = startIdx; i >= 0; --i) {
        const std::string& line = history[i];
        if (line.empty())
            continue;

        SDL_Color col = {255, 255, 255, 255};
        if (line.rfind("[Daño]", 0) == 0)
            col = {255, 220, 50, 255};
        else if (line.rfind("[Recv]", 0) == 0)
            col = {255, 100, 100, 255};
        else if (line.rfind("[PM]", 0) == 0)
            col = {80, 220, 255, 255};
        else if (line.rfind("[Clan]", 0) == 0)
            col = {150, 255, 150, 255};

        SDL_Surface* surf = TTF_RenderUTF8_Blended_Wrapped(font, line.c_str(), col, textAreaW);
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            int h = surf->h;

            currentY -= (h + 2);

            if (tex) {
                int w = surf->w;
                textures.push_back(tex);
                destRects.push_back({panelX + PADDING, currentY, w, h});
            }
            SDL_FreeSurface(surf);
            renderedMsgs++;

            if (currentY < panelY + PADDING) {
                break;
            }
        }
    }

    int maxScroll = std::max(0, totalMsgs - renderedMsgs);
    if (scrollOffset > maxScroll)
        scrollOffset = maxScroll;

    for (int i = textures.size() - 1; i >= 0; --i) {
        SDL_RenderCopy(renderer, textures[i], nullptr, &destRects[i]);
        SDL_DestroyTexture(textures[i]);
    }

    int scrollTrackX = panelX + panelW - 10;
    int scrollTrackY = panelY + PADDING;
    int scrollTrackH = bottomY - panelY - PADDING * 2;

    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect trackRect = {scrollTrackX, scrollTrackY, 6, scrollTrackH};
    SDL_RenderFillRect(renderer, &trackRect);

    if (totalMsgs > 0) {
        float visibleRatio = std::min(1.0f, (float)renderedMsgs / totalMsgs);
        int thumbH = std::max(15, (int)(scrollTrackH * visibleRatio));

        float scrollPct = maxScroll > 0 ? (float)scrollOffset / maxScroll : 0.0f;
        int thumbY = scrollTrackY + (scrollTrackH - thumbH) * (1.0f - scrollPct);

        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
        SDL_Rect thumbRect = {scrollTrackX, thumbY, 6, thumbH};
        SDL_RenderFillRect(renderer, &thumbRect);
    }

    // Separador
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawLine(renderer, panelX, bottomY, panelX + panelW, bottomY);

    if (inpTex) {
        SDL_Rect inpDest = {panelX + PADDING, bottomY + PADDING / 2, inpW, inpH};
        SDL_RenderCopy(renderer, inpTex, nullptr, &inpDest);
        SDL_DestroyTexture(inpTex);
    }

    const int handleSize = 16;
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_Rect handleRect = {panelX + panelW - handleSize, panelY, handleSize, handleSize};
    SDL_RenderFillRect(renderer, &handleRect);

    if (SDL_RectEmpty(&oldClip)) {
        SDL_RenderSetClipRect(renderer, nullptr);
    } else {
        SDL_RenderSetClipRect(renderer, &oldClip);
    }
}
