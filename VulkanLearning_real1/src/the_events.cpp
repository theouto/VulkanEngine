#include "../include/the_events.hpp"

//backport of the_events.cpp in thengine/, done as practice
namespace lve
{
  bool TheEvents::eventHandler()
  {
    for (SDL_Event event; SDL_PollEvent(&event);) 
      {
        if (event.type == SDL_EVENT_WINDOW_RESIZED)
        {
		  theWindow.resize();
        }
        if (event.type == SDL_EVENT_QUIT) 
        {
		  return false;
        }

        ImGui_ImplSDL3_ProcessEvent(&event);
      }
      return true;
  }
};
