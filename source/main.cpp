#include <iostream>

#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>

namespace {

constexpr char kWindowTitle[] = "Title";
constexpr int kStartingWinWidth = 640;
constexpr int kStartingWinHeight = 480;

SDL_Window* g_game_window = nullptr;
SDL_Renderer* g_renderer = nullptr;  // TODO(jack): Assumes single renderer/window

}

// Returns true if the application should quit.
bool HandleEvent(const SDL_Event& event) {
  switch (event.type) {
    case SDL_EVENT_QUIT:
      std::cout << "Game window closing..." << std::endl;
      return true;

    case SDL_EVENT_WINDOW_RESIZED:
      std::cout << "Window resized ("
                << event.window.data1 << ", "
                << event.window.data2 << ")" << std::endl;
      break;

    default:
      break;
  }
  return false;
}

void InitSdl() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
    return;
  }

  g_game_window = SDL_CreateWindow(kWindowTitle, kStartingWinWidth,
                                   kStartingWinHeight, SDL_WINDOW_RESIZABLE);
  if (g_game_window == nullptr) {
    std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
    return;
  }
  std::cout << "Successfully created window." << std::endl;

  g_renderer = SDL_CreateRenderer(g_game_window, nullptr);
  if (g_renderer == nullptr) {
    std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
    return;
  }
  std::cout << "Successfully created renderer." << std::endl;
}

void RunMainLoop() {
  bool running = true;
  while (running) {
    SDL_RenderClear(g_renderer);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (HandleEvent(event)) {
        running = false;
      }
    }

    SDL_RenderPresent(g_renderer);
  }
}

int main(int argc, char* argv[]) {
  InitSdl();
  RunMainLoop();
  return 0;
}
