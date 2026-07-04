#include "../include/lve_window.hpp"
//#include <stb_image.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3_image/SDL_image.h>

#define SDL_MAIN_HANDLED

#include <stdexcept>
#include <iostream>

namespace lve
{
	LveWindow::LveWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name }
	{
		initWindow();
	}

	LveWindow::~LveWindow()
	{
		SDL_DestroyWindow(window);
		SDL_Quit();
    }

	void LveWindow::initWindow()
	{
		SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);

		window = SDL_CreateWindow(windowName.c_str(), width, height, 
                                 SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

		SDL_Surface* image = IMG_Load("textures/NEEERDDDD.png");
	    SDL_SetWindowIcon(window, image);
        //SDL_SetWindowRelativeMouseMode(window, true);
    }

	void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, surface))
		{
		  throw std::runtime_error("failed to create window surface");
		}
	}
}
