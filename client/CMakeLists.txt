target_sources(argentum_online_client
    PRIVATE
    # .cpp files
    src/main.cpp
    src/Client.cpp
    src/EventHandler.cpp
    src/Game.cpp
    src/TextureManager.cpp
    src/Window.cpp

    PUBLIC
    # .h files
    src/Client.h
    src/EventHandler.h
    src/Game.h
    src/TextureManager.h
    src/Window.h
    )
