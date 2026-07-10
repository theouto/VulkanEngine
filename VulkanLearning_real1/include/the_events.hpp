#pragma once

#include "lve_window.hpp"
#include "../thirdparty/imgui/imgui_impl_sdl3.h"

//backport of the_events.hpp found in thengine/
namespace lve
{
  class TheEvents
  {
    public:
      TheEvents(LveWindow& window) : theWindow{window}{};
      ~TheEvents(){}

      bool eventHandler();

    private:
      LveWindow& theWindow;
  };
};
