# Target de testing
add_executable(argentum_online_tests)

set_target_properties(argentum_online_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

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
    TestSpawnSystem.cpp
    TestEntityManager.cpp
    TestResurrectionService.cpp
    TestInteractionService.cpp
    TestPlayerDataStore.cpp
    TestClan.cpp
    TestHealthBar.cpp
    TestViewport.cpp
    TestCharacterSprites.cpp
    TestCharacterAnimator.cpp
    TestDeath.cpp
    TestTargeting.cpp
    TestFxAnimator.cpp
    TestMotion.cpp
    TestEditorMap.cpp
    TestCamera.cpp
    TestPalette.cpp
    TestToolbar.cpp
    TestSpawnCatalogs.cpp
    TestPersistence.cpp
    TestEntityCollision.cpp
)

# Solo incluimos directorios ajenos al servidor (los del server ya los provee lib_server por herencia)
target_include_directories(argentum_online_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/editor
    ${CMAKE_BINARY_DIR}/_deps/libbcrypt-src/include
    .
)

target_link_libraries(argentum_online_tests
    PRIVATE
    lib_server
    lib_client
    lib_editor
    lib_common
    GTest::gtest_main
    GTest::gmock
)