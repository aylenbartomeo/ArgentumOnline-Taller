set_property(TARGET argentum_online_client APPEND PROPERTY AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui")

target_sources(argentum_online_client
    PRIVATE
    # .cpp files
    src/main.cpp
    src/Client.cpp
    src/EventHandler.cpp
    src/Game.cpp
    src/Receiver.cpp
    src/Sender.cpp
    src/TextureManager.cpp
    src/Window.cpp
    src/Launcher.cpp

    ui/launcher.ui

    PUBLIC
    # .h files
    src/Client.h
    src/EventHandler.h
    src/Game.h
    src/Receiver.h
    src/Sender.h
    src/TextureManager.h
    src/Window.h
    src/Launcher.h
    )
