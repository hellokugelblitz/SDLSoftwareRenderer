
cmake_minimum_required(VERSION 3.20)
project(SDLRenderer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(SDLRenderer
    source/sdl_softrender.cpp
)

# -------------------------
# Executable stuff
# -------------------------

target_include_directories(SDLRenderer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/thirdparty
)

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# -------------------------
# Link libraries
# -------------------------

target_link_libraries(SDLRenderer PRIVATE SDL3::SDL3)
