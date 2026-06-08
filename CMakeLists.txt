cmake_minimum_required(VERSION 3.24)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# Cumple con la restricción de C++20 del cliente
if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Seteamos también un estándar de C moderno para las librerías
if (NOT DEFINED CMAKE_C_STANDARD)
    set(CMAKE_C_STANDARD 11)
endif()
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_POLICY_VERSION_MINIMUM "3.5")
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

project(argentum_online
    VERSION 1.0
    HOMEPAGE_URL "https://github.com/aylenbartomeo/ArgentumOnline-Taller"
    LANGUAGES CXX
    )

add_compile_definitions(_POSIX_C_SOURCE=200809L)

option(ARGENTUM_ONLINE_TESTS "Enable / disable tests." ON)
option(ARGENTUM_ONLINE_CLIENT "Enable / disable client program." ON)
option(ARGENTUM_ONLINE_SERVER "Enable / disable server program." ON)
option(ARGENTUM_ONLINE_EDITOR "Enable / disable editor program." ON)
option(ARGENTUM_ONLINE_MAKE_WARNINGS_AS_ERRORS "Enable / disable warnings as errors." ON)

message(CMAKE_CXX_COMPILER_ID="${CMAKE_CXX_COMPILER_ID}")

# Define target: lib common between client, server and editor programs
# HEY TODO XXX: change the name to something that you like ---> DONE!!!
add_library(lib_common STATIC)

# add the source files that make the lib
# HEY!! TODO XXX: add more subfolders here
add_subdirectory(common/)

# set compiler warnings to the targets
include(cmake/CompilerWarnings.cmake)
set_project_warnings(lib_common ${ARGENTUM_ONLINE_MAKE_WARNINGS_AS_ERRORS} FALSE)

# You need to install some system packages *by hand* before compiling the
# client/editor code. In xubuntu/ubuntu 24.04 these are:
#
#   - qtbase5-dev qtbase5-dev-tools  -> required by the client launcher (find_package Qt5)
#   - the audio/font libs below      -> build dependencies of SDL2
#
# sudo apt-get install qtbase5-dev qtbase5-dev-tools libopus-dev libopusfile-dev libxmp-dev libfluidsynth-dev fluidsynth libwavpack1 libwavpack-dev libfreetype-dev wavpack
#
# SDL2, SDL2pp, toml++, bcrypt and googletest are fetched and built automatically
# by CMake (FetchContent); those do NOT need to be installed by hand.
#

# Install libSDL2 and libSDL2pp (for client or editor)
# HEY!! TODO XXX: add more dependendies (either for client, server or editor)
if(ARGENTUM_ONLINE_CLIENT OR ARGENTUM_ONLINE_EDITOR)
    include(FetchContent)
    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.30.8
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        SDL2_image
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
        GIT_TAG release-2.8.2
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        SDL2_mixer
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
        GIT_TAG release-2.8.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        SDL2_ttf
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
        GIT_TAG release-2.22.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        libSDL2pp
        OVERRIDE_FIND_PACKAGE
        URL https://github.com/libSDL2pp/libSDL2pp/archive/cc198c9a5657048bee67ece82de620b2d5661084.zip
    )

    set(SDL_PIPEWIRE OFF CACHE BOOL "" FORCE)
    set(SDL_PIPEWIRE_SHARED OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(SDL2 SDL2_image SDL2_mixer SDL2_ttf libSDL2pp)
endif()

if(ARGENTUM_ONLINE_CLIENT OR ARGENTUM_ONLINE_TESTS OR ARGENTUM_ONLINE_EDITOR OR ARGENTUM_ONLINE_SERVER)
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# Program section
# ---------------
if(ARGENTUM_ONLINE_CLIENT)
    # Buscamos Qt5 acá en la raíz para que esté disponible globalmente
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
    add_subdirectory(client)
endif()

if(ARGENTUM_ONLINE_SERVER)
    add_subdirectory(server)
endif()

if(ARGENTUM_ONLINE_EDITOR)
    add_subdirectory(editor)
endif()


# Testing section
# ---------------

if(ARGENTUM_ONLINE_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    include(GoogleTest)

    add_subdirectory(tests)
endif()
