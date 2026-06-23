#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <string>
#include <unordered_map>

#include <SDL2pp/SDL2pp.hh>

class TextureManager {
private:
    SDL2pp::Renderer& renderer;
    std::unordered_map<std::string, SDL2pp::Texture> textures;

public:
    explicit TextureManager(SDL2pp::Renderer& renderer);
    ~TextureManager() = default;

    SDL2pp::Texture& get(const std::string& path);
    SDL2pp::Texture* tryGet(const std::string& path);

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    TextureManager(TextureManager&&) = delete;
    TextureManager& operator=(TextureManager&&) = delete;
};

#endif
