#ifndef MINI_CHAT_H
#define MINI_CHAT_H

#include <deque>
#include <string>

#include <SDL2/SDL.h>
#include <SDL_ttf.h>

#include "../input/EventHandler.h"

/**
 * MiniChat: panel de mensajes superpuesto sobre el juego.
 * Muestra las últimas MAX_HISTORY líneas recibidas y el texto que el jugador está escribiendo.
 * No gestiona ningún socket; solo renderiza lo que Game le entrega.
 */
class MiniChat {
public:
    static constexpr int MAX_HISTORY = 100;
    static constexpr int FONT_SIZE = 12;
    static constexpr int PADDING = 4;

    explicit MiniChat(const std::string& fontPath);
    ~MiniChat();

    // Agrega un mensaje al historial (el más viejo se descarta si se supera MAX_LINES)
    void pushMessage(const std::string& msg);

    // Procesa el input y actualiza el estado interno (resize/scroll/toggle)
    void update(const FrameInput& input, int windowH);

    // Renderiza el panel en la esquina inferior-izquierda
    void render(SDL_Renderer* renderer, int windowW, int windowH, bool inputActive,
                const std::string& inputText);

    // Método para evitar disparar/caminar si cliqueamos el chat
    bool isMouseOver(int mx, int my, int windowH) const;
    bool isVisible() const { return visible; }

    // No permito copias
    MiniChat(const MiniChat&) = delete;
    MiniChat& operator=(const MiniChat&) = delete;

private:
    TTF_Font* font;
    std::deque<std::string> history;

    int panelW = 300;
    int panelH = 112;
    int scrollOffset = 0;
    bool visible = true;
    bool isResizing = false;
};

#endif  // MINI_CHAT_H
