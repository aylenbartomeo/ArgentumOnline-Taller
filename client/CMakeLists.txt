set_property(TARGET argentum_online_client APPEND PROPERTY AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui")

target_sources(argentum_online_client
    PRIVATE
    # .cpp files
    src/main.cpp
    src/Client.cpp
    src/EventHandler.cpp
    src/Game.cpp
    src/HealthBar.cpp
    src/Viewport.cpp
    src/CharacterSprites.cpp
    src/CharacterAnimator.cpp
    src/Death.cpp
    src/FxAnimator.cpp
    src/Targeting.cpp
    src/Receiver.cpp
    src/Sender.cpp
    src/TextureManager.cpp
    src/TileMap.cpp
    src/Window.cpp
    src/Launcher.cpp
    src/MiniChat.cpp
    src/ChatCommandParser.cpp

    ui/launcher.ui

    PUBLIC
    # .h files
    src/Client.h
    src/EventHandler.h
    src/Game.h
    src/HealthBar.h
    src/Viewport.h
    src/CharacterSprites.h
    src/CharacterAnimator.h
    src/Death.h
    src/FxAnimator.h
    src/Targeting.h
    src/Receiver.h
    src/Sender.h
    src/TextureManager.h
    src/TileMap.h
    src/Window.h
    src/Launcher.h
    )

find_package(SDL2_ttf REQUIRED)
target_link_libraries(argentum_online_client
    SDL2::SDL2
    SDL2_ttf::SDL2_ttf
)