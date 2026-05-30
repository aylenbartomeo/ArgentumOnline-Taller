target_sources(lib_common
    PRIVATE
    # .cpp files
    src/socket/socket.cpp
    src/socket/liberror.cpp
    src/socket/resolver.cpp
    src/socket/resolvererror.cpp
    src/RandomNumberGenerator.cpp
    src/protocol/Protocol.cpp
    src/OverlayRegistry.cpp
    foo.cpp
    PUBLIC
    # .h files
    src/socket/socket.h
    src/socket/liberror.h
    src/socket/resolver.h
    src/socket/resolvererror.h
    include/RandomNumberGenerator.h
    include/queue.h
    include/thread.h
    include/OverlayRegistry.h
    src/protocol/Protocol.h
    utils/position.h
    utils/types.h
    foo.h
    )

target_include_directories(lib_common
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)
