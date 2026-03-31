#include <iostream>

// SDL Includes
#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>

static const char* WINDOW_TITLE = "Title";
static const Uint32 STARTING_WIN_X = 640;
static const Uint32 STARTING_WIN_Y = 480;

static SDL_Window* GameWindow;
static SDL_Renderer* SDLRenderer; // TODO(jack): This assumes that there is only one renderer/window for the app

// SDL window handler event callback
bool HandleEvent(SDL_Event* Event)
{
	switch(Event->type) {
		case SDL_EVENT_QUIT: {
			std::cout << "Game Window Closing..." << std::endl;
			return true; // TODO(jack): use a typedef instead of false just floating here... same with below
			break;
		}
		case SDL_EVENT_WINDOW_RESIZED:  {
			Uint32 new_window_x = Event->window.data1;
			Uint32 new_window_y = Event->window.data2;
			std::cout << "Window Resize (" << new_window_x << "," << new_window_y << ")" << std::endl;
			
			break;
		}
		default: {
			// code block
		}
	}
	return false;
}

void initSDL()
{
	if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)){
		std::cout << SDL_GetError() << std::endl;
	}

	GameWindow = SDL_CreateWindow(WINDOW_TITLE, STARTING_WIN_X, STARTING_WIN_Y, SDL_WINDOW_RESIZABLE);
	if(GameWindow != 0){
		std::cout << "Successfully Created Window" << std::endl;
	}

	SDLRenderer = SDL_CreateRenderer(GameWindow, 0);
	if(SDLRenderer != 0){
		std::cout << "Successfully Created Window Renderer" << std::endl;
	}
}

void mainLoop()
{
	bool game_is_still_running = true;
	while (game_is_still_running) {
		SDL_RenderClear(SDLRenderer);  
		SDL_Event event;
		while (SDL_PollEvent(&event)) {  // poll until all events are handled!
			if(HandleEvent(&event)){
				game_is_still_running = false;
			}
    }
		SDL_RenderPresent(SDLRenderer);
	}
}

int main(int argc, char* argv[])
{
	initSDL();
	mainLoop();
}
