#include "../include/lve_window.hpp"
#include <stb_image.h>

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

		//glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		window = SDL_CreateWindow(windowName.c_str(), width, height, 
                                 SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

        //the nerd will sing again at a later date.
		SDL_Surface image;
		image.pixels = stbi_load("textures/NEEERDDDD.png", &image.w, &image.h, 0, 4); //rgba channels 
	    SDL_SetWindowIcon(window, &image);
		stbi_image_free(image.pixels);
    }

	void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, surface))
		{
          std::cout << SDL_GetError() << '\n';
		  throw std::runtime_error("failed to create window surface");
		}
	}

    bool LveWindow::eventWatcher()
    {
      for (SDL_Event event; SDL_PollEvent(&event);) 
      {
        if (event.type == SDL_EVENT_WINDOW_RESIZED)
        {
          auto lveWindow = reinterpret_cast<LveWindow*>(SDL_GetWindowFromID(event.window.windowID));
		  lveWindow->framebufferResized = true;
		  lveWindow->width = width;
		  lveWindow->height = height;
          lveWindow->windowName = windowName;
        }
        if (event.type == SDL_EVENT_QUIT) 
        {
		  return false;
        }
      }
      return true;
    }
}
