#include <iostream>
 
#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
 
// ---------------------------------------------------------------------------
// Types
// ---------------------------------------------------------------------------
 
struct Controls {
  bool up    = false;  // W or Up arrow
  bool down  = false;  // S or Down arrow
  bool left  = false;  // A or Left arrow
  bool right = false;  // D or Right arrow
};
 
// ---------------------------------------------------------------------------
// Module-level globals (internal linkage)
// ---------------------------------------------------------------------------
 
namespace {
 
constexpr char kWindowTitle[]  = "Title";
constexpr int  kBytesPerPixel  = 4;
constexpr int  kScrollSpeed    = 3;
 
int           g_win_width        = 640;
int           g_win_height       = 480;
SDL_Window*   g_game_window      = nullptr;
SDL_Renderer* g_renderer         = nullptr;  // TODO(jack): Assumes single renderer/window
SDL_Texture*  g_screen_texture   = nullptr;
void*         g_screen_pixel_data = nullptr;
Controls      g_controls;
 
}  // namespace
 
// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------
 
void ResizeTexture();
bool HandleEvent(const SDL_Event& event);
void InitSdl();
void RenderGradient(int x_off, int y_off);
void RunMainLoop();
 
// ---------------------------------------------------------------------------
// Implementation
// ---------------------------------------------------------------------------
 
void ResizeTexture() {
  if (g_screen_texture) {
    SDL_DestroyTexture(g_screen_texture);
  }
  if (g_screen_pixel_data) {
    free(g_screen_pixel_data);
  }
 
  g_screen_texture = SDL_CreateTexture(
      g_renderer, SDL_PIXELFORMAT_ARGB8888,
      SDL_TEXTUREACCESS_STREAMING, g_win_width, g_win_height);
  if (!g_screen_texture) {
    std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << std::endl;
  }
 
  g_screen_pixel_data =
      malloc(static_cast<size_t>(g_win_width * g_win_height * kBytesPerPixel));
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
      g_win_width  = event.window.data1;
      g_win_height = event.window.data2;
      ResizeTexture();
      break;
 
    case SDL_EVENT_KEY_DOWN:
      switch (event.key.key) {
        case SDLK_W: case SDLK_UP:    g_controls.up    = true; break;
        case SDLK_S: case SDLK_DOWN:  g_controls.down  = true; break;
        case SDLK_A: case SDLK_LEFT:  g_controls.left  = true; break;
        case SDLK_D: case SDLK_RIGHT: g_controls.right = true; break;
        default: break;
      }
      break;
 
    case SDL_EVENT_KEY_UP:
      switch (event.key.key) {
        case SDLK_W: case SDLK_UP:    g_controls.up    = false; break;
        case SDLK_S: case SDLK_DOWN:  g_controls.down  = false; break;
        case SDLK_A: case SDLK_LEFT:  g_controls.left  = false; break;
        case SDLK_D: case SDLK_RIGHT: g_controls.right = false; break;
        default: break;
      }
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
 
  g_game_window = SDL_CreateWindow(
      kWindowTitle, g_win_width, g_win_height, SDL_WINDOW_RESIZABLE);
  if (!g_game_window) {
    std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
    return;
  }
  std::cout << "Successfully created window." << std::endl;
 
  g_renderer = SDL_CreateRenderer(g_game_window, nullptr);
  if (!g_renderer) {
    std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
    return;
  }
  std::cout << "Successfully created renderer." << std::endl;
}
 
void RenderGradient(int x_off, int y_off) {
  auto* pixels = static_cast<uint32_t*>(g_screen_pixel_data);
  for (int y = 0; y < g_win_height; ++y) {
    for (int x = 0; x < g_win_width; ++x) {
      constexpr uint8_t kAlpha = 255;
      constexpr uint8_t kRed   = 0;
      auto green = static_cast<uint8_t>(y + y_off);  // increases downward
      auto blue  = static_cast<uint8_t>(x + x_off);  // increases rightward
      pixels[y * g_win_width + x] =
          (kAlpha << 24) | (kRed << 16) | (green << 8) | blue;
    }
  }
}
 
void RunMainLoop() {
  ResizeTexture();
 
  int x_off = 0;
  int y_off = 0;
  bool running = true;
 
  while (running) {
    // Poll events first so controls are up-to-date for this frame.
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (HandleEvent(event)) {
        running = false;
      }
    }
 
    // Apply control offsets.
    if (g_controls.up)    y_off -= kScrollSpeed;
    if (g_controls.down)  y_off += kScrollSpeed;
    if (g_controls.left)  x_off -= kScrollSpeed;
    if (g_controls.right) x_off += kScrollSpeed;
 
    // Render the gradient into the CPU-side buffer.
    RenderGradient(x_off, y_off);
 
    // Upload buffer to streaming texture.
    void* tex_pixels;
    int   pitch;
    if (SDL_LockTexture(g_screen_texture, nullptr, &tex_pixels, &pitch)) {
      memcpy(tex_pixels, g_screen_pixel_data,
             static_cast<size_t>(g_win_width * g_win_height * kBytesPerPixel));
      SDL_UnlockTexture(g_screen_texture);
    }
 
    SDL_RenderClear(g_renderer);
    SDL_RenderTexture(g_renderer, g_screen_texture, nullptr, nullptr);
    SDL_RenderPresent(g_renderer);
  }
}
 
int main(int argc, char* argv[]) {
  InitSdl();
  RunMainLoop();
  return 0;
}
