#include "../include/lve_device.hpp"
#include "../include/lve_window.hpp"
#include "../include/lve_swap_chain.hpp"

namespace lve
{
  class Imgui_LVE
  {
    public:
    
    //Imgui_LVE(LveDevice &device, LveWindow &window, LveSwapChain &swap);
    //~Imgui_LVE();
    //void init_imgui();
    //void run();

    LveSwapChain &swapChain;
    LveDevice &lveDevice;
    LveWindow &lveWindow;
  };
}
