
cmake_minimum_required(VERSION 3.20)
project(SDLRenderer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------
# Game executable
# -------------------------

target_include_directories(Game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/vendored
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/thirdparty
)

# -------------------------
# Link libraries
# -------------------------

target_link_libraries(SDLRenderer PRIVATE SDL3::SDL3)

add_executable(Game
    src/main.cpp
)


# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in vendored/SDL
# add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# Create your game executable target as usual
# add_executable(hello WIN32 hello.c)

# Link to the actual SDL3 library.

