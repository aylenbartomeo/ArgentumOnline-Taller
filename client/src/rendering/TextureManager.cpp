#include "TextureManager.h"

#include <stdexcept>

#include <SDL_image.h>

TextureManager::TextureManager(SDL2pp::Renderer& renderer): renderer(renderer) {}

SDL2pp::Texture& TextureManager::get(const std::string& path) {
    auto it = textures.find(path);
    if (it == textures.end()) {
        SDL_Texture* raw = IMG_LoadTexture(renderer.Get(), path.c_str());
        if (raw == nullptr) {
            throw std::runtime_error("No pude cargar la textura: " + path + " (" + IMG_GetError() +
                                     ")");
        }
        it = textures.emplace(path, SDL2pp::Texture(raw)).first;
    }
    return it->second;
}
