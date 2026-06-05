target_sources(argentum_online_editor
    PRIVATE
    # .cpp files
    main.cpp
    EditorMap.cpp
    Camera.cpp
    Palette.cpp
    Toolbar.cpp
    SpawnCatalogs.cpp
    Font.cpp
    Editor.cpp
    ../client/src/rendering/TextureManager.cpp
    PUBLIC
    # .h files
    EditorMap.h
    Camera.h
    Palette.h
    Toolbar.h
    SpawnCatalogs.h
    Font.h
    Editor.h
    )
