target_sources(argentum_online_client
    PRIVATE
    # .cpp files
    src/main.cpp
    src/Client.cpp
    src/EventHandler.cpp
    src/TextureManager.cpp

    PUBLIC
    # .h files
    src/Client.h
    src/EventHandler.h
    src/TextureManager.h
    )
