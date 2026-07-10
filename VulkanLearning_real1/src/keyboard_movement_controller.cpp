#include "../include/keyboard_movement_controller.hpp"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <iostream>
#include <limits>

namespace lve
{
	void KeyboardMovementController::moveInPlaneXZ(float dt, SDL_Window* window,LveGameObject& gameObject, float width, float height)
	{
		glm::vec3 rotate{ 0 };
		if (keyse[keys.lookRight]) rotate.y += 1.f;
		if (keyse[keys.lookLeft]) rotate.y -= 1.f;
		if (keyse[keys.lookUp]) rotate.x += 1.f;
		if (keyse[keys.lookDown]) rotate.x -= 1.f;
		
		float mouseX;
		float mouseY;
        glm::vec3 mousetate{0};
        SDL_MouseButtonFlags mouse = SDL_GetMouseState(&mouseX, &mouseY);
        if (mousecontrol)
        {
		    //if I want a sensitivity field then I just multiply the result below by a passed through sensitivity value
		    float rotx = height - mouseY;
		    float roty = width - mouseX;

            float divider = width*750/height;
	    	mousetate.y -= roty/divider;
      		mousetate.x += rotx/divider;
        }

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon() ||
            glm::dot(mousetate, mousetate) > std::numeric_limits<float>::epsilon()) {
			gameObject.transform.rotation += lookSpeed * (dt * rotate + mousetate * lookSpeed);
		}

		gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
		gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

		float yaw = gameObject.transform.rotation.y;
		const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
		const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
		const glm::vec3 upDir{ 0.f, -1.f, 0.f };

	    if (SDL_BUTTON_MASK(keys.rClick) & mouse) 
        {
          mousecontrol = false;
          SDL_SetWindowRelativeMouseMode(window, false);
        }
        if (SDL_BUTTON_MASK(keys.mClick) & mouse) 
        {
          mousecontrol = true;
          SDL_SetWindowRelativeMouseMode(window, true);
        }

		glm::vec3 moveDir{ 0.f };
		if (keyse[keys.moveForward]) moveDir += forwardDir;
		if (keyse[keys.moveBackward]) moveDir -= forwardDir;
		if (keyse[keys.moveRight]) moveDir += rightDir;
		if (keyse[keys.moveLeft]) moveDir -= rightDir;
		if (keyse[keys.moveUp]) moveDir += upDir;
		if (keyse[keys.moveDown]) moveDir -= upDir;

		if (keyse[keys.close]) SDL_Quit();

		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
		{
			gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
		}
	}
}
