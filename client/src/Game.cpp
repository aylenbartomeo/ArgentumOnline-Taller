#include "Game.h"

#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

#include <SDL2/SDL.h>

#include "common/include/dto/StartMoveDTO.h"

namespace {
constexpr int TILE_SIZE = 32;
constexpr int WINDOW_WIDTH = 640;
constexpr int WINDOW_HEIGHT = 480;
constexpr Uint32 MOVE_INTERVAL_MS = 200;

// Carpeta de recursos relativa a la raiz del proyecto (desde donde se corre el
// binario: ./build/argentum_online_client).
constexpr const char* RESOURCES_DIR = "client/resources/";

// Tiny Town no trae sprites de personaje por id, asi que todos los jugadores se
// dibujan con este tile del tileset y el propio se distingue con un borde.
constexpr int PLAYER_SPRITE = 104;

std::string readWholeFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("No pude abrir el archivo: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
}  // namespace

Game::Game(Client& client):
        sdl(SDL_INIT_VIDEO),
        window("Argentum Online - Client", WINDOW_WIDTH, WINDOW_HEIGHT),
        events(),
        client(client),
        textures(window.getRenderer()),
        map(readWholeFile(std::string(RESOURCES_DIR) + "default.json")),
        lastSnapshot(),
        lastMoveSentMs(0) {
    // Escalado nearest-neighbor para que el pixel art no se vea borroso.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    // El mundo se dibuja siempre a 640x480 y SDL lo escala si agrandan la ventana.
    window.getRenderer().SetLogicalSize(WINDOW_WIDTH, WINDOW_HEIGHT);
}

void Game::run() {
    while (true) {
        const FrameInput input = events.pollEvents();
        if (input.quit) {
            break;
        }
        sendMoveIfDue(input);
        render();
        SDL_Delay(16);
    }
}

void Game::sendMoveIfDue(const FrameInput& input) {
    const Uint32 now = SDL_GetTicks();
    if (now - lastMoveSentMs < MOVE_INTERVAL_MS) {
        return;
    }

    std::optional<Movement> direction;
    if (input.moveNorth)
        direction = Movement::UP;
    else if (input.moveSouth)
        direction = Movement::DOWN;
    else if (input.moveEast)
        direction = Movement::RIGHT;
    else if (input.moveWest)
        direction = Movement::LEFT;

    if (direction) {
        client.sendCommand(StartMoveDTO(*direction));
        lastMoveSentMs = now;
    }
}

void Game::render() {
    SnapshotDTO incoming;
    while (client.tryPopSnapshot(incoming)) {
        lastSnapshot = incoming;
    }

    SDL2pp::Renderer& renderer = window.getRenderer();
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();

    renderTerrain();
    renderEntities();

    renderer.Present();
}

void Game::renderTerrain() {
    SDL2pp::Renderer& renderer = window.getRenderer();
    SDL2pp::Texture& tileset = textures.get(std::string(RESOURCES_DIR) + map.getTileset());
    const int src = map.getTileSize();
    const int cols = map.getTilesetCols();

    for (int row = 0; row < map.getHeight(); ++row) {
        for (int col = 0; col < map.getWidth(); ++col) {
            const int id = map.tileAt(col, row);
            const SDL2pp::Rect srcRect((id % cols) * src, (id / cols) * src, src, src);
            const SDL2pp::Rect dstRect(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE);
            renderer.Copy(tileset, srcRect, dstRect);
        }
    }
}

void Game::renderEntities() {
    SDL2pp::Renderer& renderer = window.getRenderer();
    SDL2pp::Texture& tileset = textures.get(std::string(RESOURCES_DIR) + map.getTileset());
    const int src = map.getTileSize();
    const int cols = map.getTilesetCols();
    const uint32_t myId = client.getClientId();

    for (const EntityDTO& entity: lastSnapshot.entities) {
        const SDL2pp::Rect srcRect((PLAYER_SPRITE % cols) * src, (PLAYER_SPRITE / cols) * src, src,
                                   src);
        const SDL2pp::Rect dstRect(entity.x * TILE_SIZE, entity.y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
        renderer.Copy(tileset, srcRect, dstRect);

        if (entity.id == myId) {
            renderer.SetDrawColor(255, 235, 0, 255);
            renderer.DrawRect(dstRect);
        }
    }
}
