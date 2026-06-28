#include "../include/keyboard_movement_controller.hpp"

#include <SDL3/SDL_events.h>
#include <iostream>
#include <limits>

namespace lve
{
	void KeyboardMovementController::moveInPlaneXZ(SDL_Window* window, float dt, LveGameObject& gameObject, float oMouseX, float oMouseY) 
	{
        SDL_PumpEvents();
		glm::vec3 rotate{ 0 };
		if (keyse[keys.lookRight] == SDLK_DOWN) rotate.y += 1.f;
		if (keyse[keys.lookLeft] == SDLK_DOWN) rotate.y -= 1.f;
		if (keyse[keys.lookUp] == SDLK_DOWN) rotate.x += 1.f;
		if (keyse[keys.lookDown] == SDLK_DOWN) rotate.x -= 1.f;
		
		float mouseX;
		float mouseY;
        if (mousecontrol)
        {
		    SDL_GetMouseState(&mouseX, &mouseY);
		
		    //if I want a sensitivity field then I just multiply the result below by a passed through sensitivity value
		    float rotx = (float)(mouseY - oMouseY);
		    float roty = (float)(mouseX - oMouseX);			

	    	rotate.y += roty;
      		rotate.x -= rotx;
        }

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
			gameObject.transform.rotation += lookSpeed * dt * rotate;
		}

		gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
		gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

		float yaw = gameObject.transform.rotation.y;
		const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
		const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
		const glm::vec3 upDir{ 0.f, -1.f, 0.f };

		

		glm::vec3 moveDir{ 0.f };
		if (keyse[keys.moveForward] == SDLK_DOWN) moveDir += forwardDir;
		if (keyse[keys.moveBackward] == SDLK_DOWN) moveDir -= forwardDir;
		if (keyse[keys.moveRight] == SDLK_DOWN) moveDir += rightDir;
		if (keyse[keys.moveLeft] == SDLK_DOWN) moveDir -= rightDir;
		if (keyse[keys.moveUp] == SDLK_DOWN) moveDir += upDir;
		if (keyse[keys.moveDown] == SDLK_DOWN) moveDir -= upDir;
        /*
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) 
        {
          mousecontrol = false;
          glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS) 
        {
          mousecontrol = true;
          glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);        
        }
        */

		if (keyse[keys.close]) SDL_Quit();

		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
		{
			gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
		}
	}
}
