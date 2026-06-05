set_property(TARGET argentum_online_client APPEND PROPERTY AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui")

target_sources(argentum_online_client
    PRIVATE
    # core
    src/core/main.cpp
    src/core/Client.cpp
    src/core/Game.cpp
    src/core/Targeting.cpp

    # network
    src/network/Sender.cpp
    src/network/Receiver.cpp

    # ui
    src/ui/Window.cpp
    src/ui/Launcher.cpp
    src/ui/MiniChat.cpp
    src/ui/HealthBar.cpp

    # rendering
    src/rendering/TileMap.cpp
    src/rendering/Viewport.cpp
    src/rendering/TextureManager.cpp

    # animation
    src/animation/CharacterAnimator.cpp
    src/animation/CharacterSprites.cpp
    src/animation/Death.cpp
    src/animation/FxAnimator.cpp

    # input
    src/input/EventHandler.cpp
    src/input/ChatCommandParser.cpp

    ui/launcher.ui

    PUBLIC
    # core
    src/core/Client.h
    src/core/Game.h
    src/core/Targeting.h

    # network
    src/network/Sender.h
    src/network/Receiver.h

    # ui
    src/ui/Window.h
    src/ui/Launcher.h
    src/ui/MiniChat.h
    src/ui/HealthBar.h

    # rendering
    src/rendering/TileMap.h
    src/rendering/Viewport.h
    src/rendering/TextureManager.h

    # animation
    src/animation/CharacterAnimator.h
    src/animation/CharacterSprites.h
    src/animation/Death.h
    src/animation/FxAnimator.h

    # input
    src/input/EventHandler.h
    src/input/ChatCommandParser.h
)

# Permite includes con rutas relativas entre módulos (../network/Sender.h, etc.)
target_include_directories(argentum_online_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

find_package(SDL2_ttf REQUIRED)
target_link_libraries(argentum_online_client
    SDL2::SDL2
    SDL2_ttf::SDL2_ttf
)