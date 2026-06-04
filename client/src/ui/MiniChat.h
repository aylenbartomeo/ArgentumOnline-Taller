#ifndef MINI_CHAT_H
#define MINI_CHAT_H

#include <deque>
#include <string>

#include <SDL2/SDL.h>
#include <SDL_ttf.h>

/**
 * MiniChat: panel de mensajes superpuesto sobre el juego.
 * Muestra las últimas MAX_LINES líneas recibidas y el texto que el jugador está escribiendo.
 * No gestiona ningún socket; solo renderiza lo que Game le entrega.
 */
class MiniChat {
public:
    static constexpr int MAX_LINES = 6;
    static constexpr int PANEL_W = 300;
    static constexpr int PANEL_H = 112;
    static constexpr int LINE_H = 15;
    static constexpr int FONT_SIZE = 12;
    static constexpr int PADDING = 4;

    explicit MiniChat(const std::string& fontPath);
    ~MiniChat();

    // Agrega un mensaje al historial (el más viejo se descarta si se supera MAX_LINES)
    void pushMessage(const std::string& msg);

    // Renderiza el panel en la esquina inferior-izquierda
    void render(SDL_Renderer* renderer, int windowW, int windowH, bool inputActive,
                const std::string& inputText);

    // No permito copias
    MiniChat(const MiniChat&) = delete;
    MiniChat& operator=(const MiniChat&) = delete;

private:
    TTF_Font* font;
    std::deque<std::string> history;  // historial de mensajes recientes

    // Dibuja una línea de texto con fondo semi-transparente
    void renderLine(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color);
};

#endif  // MINI_CHAT_H
