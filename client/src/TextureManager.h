#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

class TextureManager
{
private:
    /* data */
public:
    TextureManager(/* args */);

    /* No permito copias */
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    /* Permito movimientos */
    TextureManager(TextureManager&&) = default;
    TextureManager& operator=(TextureManager&&) = default;

    ~TextureManager();
};

#endif
