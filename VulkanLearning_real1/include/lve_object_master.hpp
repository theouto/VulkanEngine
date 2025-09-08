#pragma once

#include "lve_game_object.hpp"
#include "lve_textures.hpp"

#include <vector>


//The point of the game object master will be to streamline the process of having game objects,
//also easing the process of having objects with different textures in the scene while managing their material properties
//as not all assets will have the material assets needed to fully utilise the programmed shaders
//
//This is written here as it is now because I want to pressure myself to finish this at some point, and this looks silly

namespace lve
{
	class ObjectMaster
	{
		public:
		
		private:
			std::vector<LveTextures> textures;
	};
}
