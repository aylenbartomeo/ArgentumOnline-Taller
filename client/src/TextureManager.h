#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <string>
#include <unordered_map>

#include <SDL2pp/SDL2pp.hh>

/**
 * Carga y cachea texturas por ruta de archivo. Una imagen se lee del disco una
 * sola vez: los pedidos siguientes devuelven la textura ya cargada.
 */
class TextureManager {
private:
    SDL2pp::Renderer& renderer;
    std::unordered_map<std::string, SDL2pp::Texture> textures;

public:
    explicit TextureManager(SDL2pp::Renderer& renderer);
    ~TextureManager() = default;

    // Devuelve la textura de 'path', cargandola del disco la primera vez.
    SDL2pp::Texture& get(const std::string& path);

    /* No permito copias ni movimientos: tiene una referencia al renderer. */
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    TextureManager(TextureManager&&) = delete;
    TextureManager& operator=(TextureManager&&) = delete;
};

#endif
