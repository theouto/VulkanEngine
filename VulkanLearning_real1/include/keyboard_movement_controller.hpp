#pragma once

#include <SDL3/SDL_mouse.h>
#define GLM_ENABLE_EXPERIMENTAL

#include "lve_game_object.hpp"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace lve
{
	class KeyboardMovementController
	{
	public:
		struct KeyMappings
		{
            int moveLeft = SDL_SCANCODE_A;
            int moveRight = SDL_SCANCODE_D;
            int moveForward = SDL_SCANCODE_W;
            int moveBackward = SDL_SCANCODE_S;
            int moveUp = SDL_SCANCODE_E;
            int moveDown = SDL_SCANCODE_Q;
            int lookLeft = SDL_SCANCODE_LEFT;
            int lookRight = SDL_SCANCODE_RIGHT;
            int lookUp = SDL_SCANCODE_UP;
            int lookDown = SDL_SCANCODE_DOWN;
            int close = SDL_SCANCODE_ESCAPE;

            int rClick = SDL_BUTTON_RIGHT;
            int mClick = SDL_BUTTON_MIDDLE;
		};

        void moveInPlaneXZ(float dt, SDL_Window* window,LveGameObject &gameObject, float width, float height);
        bool mousecontrol;

        const bool* keyse = SDL_GetKeyboardState(nullptr);
        KeyMappings keys{};
        float moveSpeed{ 3.f };
        float lookSpeed{ 1.0f };
	};
}
