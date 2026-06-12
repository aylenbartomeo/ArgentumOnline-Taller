find_package(SDL2_ttf REQUIRED)

add_library(lib_client STATIC)

# Crucial para Qt5: Activamos el procesado automático de UI y Meta-Objetos en la lib
set_target_properties(lib_client PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

set_property(TARGET lib_client APPEND PROPERTY AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui")

target_sources(lib_client
    PRIVATE
    # core
    src/core/Client.cpp
    src/core/Game.cpp
    src/core/Targeting.cpp
    src/core/systems/AudioSystem.cpp
    src/core/systems/CameraSystem.cpp
    src/core/systems/FxSystem.cpp
    src/core/systems/EntityRenderer.cpp
    src/core/systems/WorldRenderer.cpp
    src/core/systems/StateAudioTrigger.cpp
    src/core/systems/InputProcessor.cpp

    # network
    src/network/Sender.cpp
    src/network/Receiver.cpp

    # ui
    src/ui/Window.cpp
    src/ui/Launcher.cpp
    src/ui/MiniChat.cpp
    src/ui/HealthBar.cpp
    src/ui/HudLayout.cpp
    src/ui/ItemIcons.cpp
    src/ui/HudPanel.cpp
    src/ui/ManualPanel.cpp

    # rendering
    src/rendering/TileMap.cpp
    src/rendering/IndoorRegions.cpp
    src/rendering/Viewport.cpp
    src/rendering/TextureManager.cpp

    # animation
    src/animation/CharacterAnimator.cpp
    src/animation/CharacterSprites.cpp
    src/animation/Death.cpp
    src/animation/FxAnimator.cpp
    src/animation/Motion.cpp
    src/animation/ProjectileAnimator.cpp

    # input
    src/input/EventHandler.cpp
    src/input/ChatCommandParser.cpp

    ui/launcher.ui

    PUBLIC
    # core
    src/core/Client.h
    src/core/Game.h
    src/core/Targeting.h
    src/core/systems/AudioSystem.h
    src/core/systems/CameraSystem.h
    src/core/systems/FxSystem.h
    src/core/systems/EntityRenderer.h
    src/core/systems/WorldRenderer.h
    src/core/systems/StateAudioTrigger.h
    src/core/systems/InputProcessor.h
    src/core/common/GameConstants.h
    src/core/common/FxType.h
    src/core/common/WeaponHelper.h

    # network
    src/network/Sender.h
    src/network/Receiver.h

    # ui
    src/ui/Window.h
    src/ui/Launcher.h
    src/ui/MiniChat.h
    src/ui/HealthBar.h
    src/ui/HudLayout.h
    src/ui/ItemIcons.h
    src/ui/HudPanel.h
    src/ui/ManualPanel.h

    # rendering
    src/rendering/TileMap.h
    src/rendering/Viewport.h
    src/rendering/TextureManager.h
    src/rendering/NpcVisuals.h

    # animation
    src/animation/CharacterAnimator.h
    src/animation/CharacterSprites.h
    src/animation/Death.h
    src/animation/FxAnimator.h
    src/animation/Motion.h

    # input
    src/input/EventHandler.h
    src/input/ChatCommandParser.h
    src/input/NpcPolicy.h
)

# Definimos que las carpetas del cliente se expongan públicamente 
# para que los tests e hilos secundarios hereden los includes automáticamente.
target_include_directories(lib_client PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/network
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rendering
    ${CMAKE_CURRENT_SOURCE_DIR}/src/animation
    ${CMAKE_CURRENT_SOURCE_DIR}/src/input
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/systems
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/common
    PRIVATE
    ${libSDL2pp_SOURCE_DIR}
)

# Vinculamos las librerías gráficas y Qt5 a la librería estática
target_link_libraries(lib_client PUBLIC
    lib_common
    SDL2pp::SDL2pp
    SDL2_image::SDL2_image
    SDL2::SDL2
    SDL2_ttf::SDL2_ttf
    SDL2_mixer::SDL2_mixer
    Qt5::Widgets 
    nlohmann_json::nlohmann_json
    pthread
)

add_executable(argentum_online_client src/core/main.cpp)

# Forzamos a que el ejecutable también se guarde en la raíz del build (como pediste)
set_target_properties(argentum_online_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_link_libraries(argentum_online_client PRIVATE lib_client)

set_project_warnings(argentum_online_client ${ARGENTUM_ONLINE_MAKE_WARNINGS_AS_ERRORS} FALSE)