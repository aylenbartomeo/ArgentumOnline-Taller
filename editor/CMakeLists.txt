target_sources(argentum_online_editor
    PRIVATE
    # .cpp files
    main.cpp
    EditorMap.cpp
    Camera.cpp
    Palette.cpp
    Toolbar.cpp
    SpawnCatalogs.cpp
    BitmapFont.cpp
    Editor.cpp
    ../client/src/TextureManager.cpp
    PUBLIC
    # .h files
    EditorMap.h
    Camera.h
    Palette.h
    Toolbar.h
    SpawnCatalogs.h
    BitmapFont.h
    CharRect.h
    Editor.h
    )
