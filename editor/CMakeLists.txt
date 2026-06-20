add_library(lib_editor STATIC)

target_sources(lib_editor
    PRIVATE
    # .cpp files
    EditorMap.cpp
    Camera.cpp
    Palette.cpp
    Toolbar.cpp
    EditorLayout.cpp
    MapRenderer.cpp
    CatalogSprites.cpp
    ScreenEditor.cpp
    SpawnCatalogs.cpp
    Font.cpp
    Editor.cpp
    CityStamp.cpp
    DungeonStamp.cpp
    ForestStamp.cpp
    DesertStamp.cpp
    SmartEraser.cpp
    BlockBrush.cpp
    MapChooser.cpp
    PUBLIC
    # .h files
    EditorMap.h
    Camera.h
    Palette.h
    Toolbar.h
    EditorLayout.h
    MapRenderer.h
    CatalogSprites.h
    ScreenEditor.h
    SpawnCatalogs.h
    Font.h
    Editor.h
    CityStamp.h
    DungeonStamp.h
    ForestStamp.h
    DesertStamp.h
    SmartEraser.h
    BlockBrush.h
    MapChooser.h
    MapDefaults.h
    )

# Exponemos los directorios de inclusión del editor públicamente
target_include_directories(lib_editor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(lib_editor PUBLIC
    lib_common
    lib_client
    SDL2pp::SDL2pp
    SDL2_image::SDL2_image
    SDL2_ttf
    nlohmann_json::nlohmann_json
    pthread
)

add_executable(argentum_online_editor MainScreenEditor.cpp)

set_target_properties(argentum_online_editor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_link_libraries(argentum_online_editor PRIVATE lib_editor)

set_project_warnings(argentum_online_editor ${ARGENTUM_ONLINE_MAKE_WARNINGS_AS_ERRORS} FALSE)

add_executable(argentum_online_editor_classic main.cpp)

set_target_properties(argentum_online_editor_classic PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_link_libraries(argentum_online_editor_classic PRIVATE lib_editor)

set_project_warnings(argentum_online_editor_classic ${ARGENTUM_ONLINE_MAKE_WARNINGS_AS_ERRORS} FALSE)

add_executable(argentum_map_builder MapBuilder.cpp)

set_target_properties(argentum_map_builder PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_link_libraries(argentum_map_builder PRIVATE lib_editor)

set_project_warnings(argentum_map_builder ${ARGENTUM_ONLINE_MAKE_WARNINGS_AS_ERRORS} FALSE)