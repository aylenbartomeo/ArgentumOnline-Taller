#include "TextureManager.h"

#include <stdexcept>

#include <SDL_image.h>

TextureManager::TextureManager(SDL2pp::Renderer& renderer): renderer(renderer) {}

SDL2pp::Texture* TextureManager::tryGet(const std::string& path) {
    auto it = textures.find(path);
    if (it == textures.end()) {
        SDL_Texture* raw = IMG_LoadTexture(renderer.Get(), path.c_str());
        if (raw == nullptr) {
            return nullptr;
        }
        it = textures.emplace(path, SDL2pp::Texture(raw)).first;
    }
    return &it->second;
}

SDL2pp::Texture& TextureManager::get(const std::string& path) {
    SDL2pp::Texture* tex = tryGet(path);
    if (tex == nullptr) {
        throw std::runtime_error("No pude cargar la textura: " + path + " (" + IMG_GetError() +
                                 ")");
    }
    return *tex;
}
