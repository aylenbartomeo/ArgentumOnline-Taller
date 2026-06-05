# Target de testing
add_executable(argentum_online_tests)

target_sources(argentum_online_tests
    PRIVATE
    TestProtocol.cpp
    TestServerArchitecture.cpp
    TestMap.cpp
    TestWorld.cpp
    TestGameLoop.cpp
    TestCombat.cpp
    TestAuthManager.cpp
    TestEquipmentComponent.cpp
    TestInventoryComponent.cpp
    TestStatsComponent.cpp
    TestRegenerationComponent.cpp
    TestTileMap.cpp
    TestMerchant.cpp
    TestPriest.cpp
    TestGlobalBank.cpp
    TestBanker.cpp
    TestGroundItemLayer.cpp
    TestSafeZoneLayer.cpp
    TestNPCLayer.cpp
    TestPlayer.cpp
    TestConsumable.cpp
    TestPlayerDataStore.cpp
    TestClan.cpp
    TestHealthBar.cpp
    TestViewport.cpp
    TestCharacterSprites.cpp
    TestCharacterAnimator.cpp
    TestEditorMap.cpp
    TestCamera.cpp
    TestPalette.cpp
    TestToolbar.cpp
    TestSpawnCatalogs.cpp
    TestPersistence.cpp

    # ARMAR UN lib_client
    ../client/src/rendering/TileMap.cpp
    ../client/src/ui/HealthBar.cpp
    ../client/src/rendering/Viewport.cpp
    ../client/src/animation/CharacterSprites.cpp
    ../client/src/animation/CharacterAnimator.cpp
    ../editor/EditorMap.cpp
    ../editor/Camera.cpp
    ../editor/Palette.cpp
    ../editor/Toolbar.cpp
    ../editor/SpawnCatalogs.cpp
)

# Solo incluimos directorios ajenos al servidor (los del server ya los provee lib_server por herencia)
target_include_directories(argentum_online_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/common/utils
    ${CMAKE_SOURCE_DIR}/client/src
    ${CMAKE_SOURCE_DIR}/editor
    ${CMAKE_BINARY_DIR}/_deps/libbcrypt-src/include
    .
)

target_link_libraries(argentum_online_tests
    PRIVATE
    lib_server
    lib_common
    GTest::gtest_main
    GTest::gmock
)