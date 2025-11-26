#pragma once

#include "lve_window.hpp"
#include "lve_device.hpp"
#include "lve_model.hpp"
#include "lve_game_object.hpp"
#include "lve_renderer.hpp"
#include "lve_descriptors.hpp"
#include "lve_textures.hpp"

#include <memory>
#include <vector>

namespace lve
{
	class FirstApp
	{
	public:
		static constexpr int WIDTH = 1920;
		static constexpr int HEIGHT = 1080;


		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;

		void run();

	private:
		void loadGameObjects();

		LveWindow lveWindow{ WIDTH, HEIGHT, "thengine" };

		LveDevice lveDevice{ lveWindow };
		LveRenderer lveRenderer{ lveWindow, lveDevice };
        std::unique_ptr<LveDescriptorSetLayout> matLayout;

		std::unique_ptr<LveDescriptorPool> globalPool{};
		LveGameObject::Map gameObjects;
	};
}
