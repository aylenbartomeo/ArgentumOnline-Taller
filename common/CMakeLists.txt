target_sources(lib_common
    PRIVATE
    # .cpp files
    socket/socket.cpp
    socket/liberror.cpp
    socket/resolver.cpp
    socket/resolvererror.cpp
    foo.cpp
    PUBLIC
    # .h files
    socket/socket.h
    socket/liberror.h
    socket/resolver.h
    socket/resolvererror.h
    queue.h
    thread.h
    foo.h
    )
