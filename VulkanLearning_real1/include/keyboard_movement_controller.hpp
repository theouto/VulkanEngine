#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include "lve_game_object.hpp"
#include "lve_window.hpp"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace lve
{
	class KeyboardMovementController
	{
	public:
		struct KeyMappings
		{
            int moveLeft = SDLK_A;
            int moveRight = SDLK_D;
            int moveForward = SDLK_W;
            int moveBackward = SDLK_S;
            int moveUp = SDLK_E;
            int moveDown = SDLK_Q;
            int lookLeft = SDLK_LEFT;
            int lookRight = SDLK_RIGHT;
            int lookUp = SDLK_UP;
            int lookDown = SDLK_DOWN;
            int close = SDLK_ESCAPE;
		};

        void moveInPlaneXZ(SDL_Window* window, float dt, LveGameObject &gameObject, float oMouseX, float oMouseY);
        bool mousecontrol;

        const bool* keyse = SDL_GetKeyboardState(nullptr);
        KeyMappings keys{};
        float moveSpeed{ 3.f };
        float lookSpeed{ 1.0f };
	};
}
