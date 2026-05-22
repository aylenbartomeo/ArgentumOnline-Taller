target_sources(argentum_online_tests
    PRIVATE
    TestProtocol.cpp
    TestServerArchitecture.cpp
    TestMap.cpp
    TestWorld.cpp
    TestGameLoop.cpp
    TestAuthManager.cpp
    ../server/src/ConnectionMonitor.cpp
    ../server/src/GameLoop.cpp
    ../server/src/Map.cpp
    ../server/src/World.cpp
    ../server/src/auth/AuthManager.cpp
)

target_include_directories(argentum_online_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/server/include
    ${CMAKE_SOURCE_DIR}/server/src
    ${CMAKE_SOURCE_DIR}/server/src/model
    ${CMAKE_SOURCE_DIR}/server/src/model/entities
    ${CMAKE_SOURCE_DIR}/server/src/model/items
    ${CMAKE_SOURCE_DIR}/server/src/model/inventory
    ${CMAKE_BINARY_DIR}/_deps/libbcrypt-src/include
    .
)

target_link_libraries(argentum_online_tests
    tomlplusplus::tomlplusplus
    bcrypt
)
