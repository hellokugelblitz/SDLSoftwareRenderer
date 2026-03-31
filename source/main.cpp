#include <iostream>

// SDL Includes
#include <SDL3/SDL.h>

int main(){
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Title", "Message", 0);
	std::cout << "Build Test" << std::endl;
}
