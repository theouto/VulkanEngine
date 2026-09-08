#pragma once

#include "lve_window.hpp"
#include "lve_model.hpp"
#include "../thirdparty/imgui/imgui_impl_sdl3.h"
#include <memory>

//backport of the_events.hpp found in thengine/
namespace lve
{
  class TheEvents
  {
    public:
      TheEvents(LveWindow& window) : theWindow{window}{toUpdate.resize(0);};
      ~TheEvents(){}

      void updateModels();
      void addToUpdate(std::shared_ptr<LveModel> model) {toUpdate.push_back(model);}
      bool eventHandler();

    private:
      LveWindow& theWindow;
      std::vector<std::shared_ptr<LveModel>> toUpdate;
  };
};
