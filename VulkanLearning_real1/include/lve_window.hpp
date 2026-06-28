#pragma once

//#define GLFW_INCLUDE_VULKAN
//
#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_mouse.h>
#include <string>

namespace lve
{
	class LveWindow
	{
	public:
		LveWindow(int w, int h, std::string name);
		~LveWindow();

		LveWindow(const LveWindow&) = delete;
		LveWindow &operator=(const LveWindow&) = delete;
 
        bool cosa;

		bool shouldClose() { return false; }
		VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
		bool wasWindowResized() { return framebufferResized; };
		void resetWindowResizedFlag() { framebufferResized = false; };
		SDL_Window *getGLFWwindow() const { return window; }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
        bool eventWatcher();

	private:
		void initWindow();

		int width;
		int height;
		bool framebufferResized = false;

		std::string windowName;
		SDL_Window* window;
	};
}
