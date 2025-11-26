#include "../include/first_app.hpp"

#include "../include/keyboard_movement_controller.hpp"
#include "../include/lve_camera.hpp"
#include "../include/lve_buffer.hpp"
#include "../include/lve_frame_info.hpp"
#include "../systems/point_light_system.hpp"
#include "../systems/simple_render_system.hpp"
#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>
#include <stdexcept>
#include <chrono>
#include <array>

namespace lve
{
	FirstApp::FirstApp()
    {
        globalPool = LveDescriptorPool::Builder(lveDevice)
            .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT * LveGameObject::MAX_OBJECTS * 2)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT * LveGameObject::MAX_OBJECTS * 2)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, LveSwapChain::MAX_FRAMES_IN_FLIGHT * LveGameObject::MAX_OBJECTS * 4)
            .build();
        loadGameObjects();
    }

	FirstApp::~FirstApp() {}


	void FirstApp::run()
	{
        /*
        std::unique_ptr<LveTextures> texture = std::make_unique<LveTextures>( lveDevice, 
            "textures/PavingStones115C_2K-PNG_Color.png", LveTextures::COLOR);

        std::unique_ptr<LveTextures> specular = std::make_unique<LveTextures>( lveDevice, 
            "textures/PavingStones115C_2K-PNG_Roughness.png", LveTextures::SPECULAR );
        
        std::unique_ptr<LveTextures> normal = std::make_unique<LveTextures>( lveDevice,
            "textures/PavingStones115C_2K-PNG_NormalGL.png", LveTextures::NORMAL);
        
        std::unique_ptr<LveTextures> displacement = std::make_unique<LveTextures>( lveDevice,
            "textures/PavingStones115C_2K-PNG_Displacement.png", LveTextures::DEPTH);

        //LveTextures metalness{ lveDevice, "textures/PavingStones115C_2K-PNG_Reflectiveness.png", VK_FORMAT_R8_UNORM };
        */

        std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++)
        {
            uboBuffers[i] = std::make_unique<LveBuffer>(
                lveDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        //I will fix this garbage
        auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)            
            //.addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
            .build();   

        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++)
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();

            /*
            auto imageInfo = texture->getDescriptorInfo();
            auto specInfo = specular->getDescriptorInfo();
            auto nomInfo = normal->getDescriptorInfo();
            auto dispInfo = displacement->getDescriptorInfo();
            */

            LveDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                /*
                .writeImage(1, &imageInfo)
                .writeImage(2, &specInfo)
                .writeImage(3, &nomInfo)
                .writeImage(4, &dispInfo)
                */
                .build(globalDescriptorSets[i]);
        }

		SimpleRenderSystem simpleRenderSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        PointLightSystem pointLightSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
        LveCamera camera{};

        auto viewerObject = LveGameObject::createGameObject();
        viewerObject.transform.translation.z = -1.5f;
	

	        // https://www.glfw.org/docs/3.3/input_guide.html#raw_mouse_motion <- important
        glfwSetInputMode(lveWindow.getGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported())
        {
            glfwSetInputMode(lveWindow.getGLFWwindow(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }     

        KeyboardMovementController cameraController{};

        double mouseX = 0.f;
        double mouseY = 0.f;
	
	auto currentTime = std::chrono::high_resolution_clock::now();
	
	while (!lveWindow.shouldClose())
	{
	    glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            //std::cout << "Framerate: " << 1 / frameTime << '\n';
            currentTime = newTime;
            
            cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject, mouseX, mouseY);
            glfwGetCursorPos(lveWindow.getGLFWwindow(), &mouseX, &mouseY);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
	

            float aspect = lveRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.01f, 30.f);
		

		if (auto commandBuffer = lveRenderer.beginFrame())
		{		
			int frameIndex = lveRenderer.getFrameIndex();
                	FrameInfo frameInfo
                	{
                 		frameIndex,
                		frameTime,
                    		commandBuffer,
                    		camera,
                    		globalDescriptorSets[frameIndex],
                    		gameObjects
                	};

		
                //update               
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();    

                //render
		        lveRenderer.beginSwapChainRenderPass(commandBuffer);
				
                //order is important!
                simpleRenderSystem.renderGameObjects(frameInfo);
                
                pointLightSystem.render(frameInfo);
                
                lveRenderer.endSwapChainRenderPass(commandBuffer);
				lveRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(lveDevice.device());
	}

	void FirstApp::loadGameObjects()
	{
       std::shared_ptr<LveTextures> texture = std::make_shared<LveTextures>( lveDevice, 
            "textures/PavingStones115C_2K-PNG_Color.png", LveTextures::COLOR);

        std::shared_ptr<LveTextures> specular = std::make_shared<LveTextures>( lveDevice, 
            "textures/PavingStones115C_2K-PNG_Roughness.png", LveTextures::SPECULAR );
        
        std::shared_ptr<LveTextures> normal = std::make_shared<LveTextures>( lveDevice,
            "textures/PavingStones115C_2K-PNG_NormalGL.png", LveTextures::NORMAL);
        
        std::shared_ptr<LveTextures> displacement = std::make_shared<LveTextures>( lveDevice,
            "textures/PavingStones115C_2K-PNG_Displacement.png", LveTextures::DEPTH);

         
        std::vector<std::shared_ptr<LveTextures>> texteres;
        texteres.push_back(texture);
        texteres.push_back(specular);
        texteres.push_back(normal);
        texteres.push_back(displacement);
        
        auto matLayout = LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();
        
        std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "models/pleasepot.obj");
        auto gameObj = LveGameObject::createGameObject();
        gameObj.model = lveModel;
        gameObj.transform.translation = { .0f, .5f, 0.f };
        gameObj.transform.scale = { .25f, -.25f, .25f };
        gameObj.textures = texteres;
        gameObjects.emplace(gameObj.getId(), std::move(gameObj));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/smooth_vase.obj");
        auto sVase = LveGameObject::createGameObject();
        sVase.model = lveModel;
        sVase.transform.translation = { -.5f, .0f, 0.f };
        sVase.transform.scale = { 1.f, 1.f, 1.f };
        sVase.textures = texteres;
        gameObjects.emplace(sVase.getId(), std::move(sVase));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/flat_vase.obj");
        auto vase = LveGameObject::createGameObject();
        vase.model = lveModel;
        vase.transform.translation = { .5f, .0f, 0.f };
        vase.transform.scale = { 1.f, 1.f, 1.f };
        vase.textures = texteres;
        gameObjects.emplace(vase.getId(), std::move(vase));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/quad.obj");
        auto quad = LveGameObject::createGameObject();
        quad.model = lveModel;
        quad.transform.translation = { 0.f, .5f, 0.f };
        quad.transform.scale = { 3.f, 1.f, 3.f };
        quad.textures = texteres;
        gameObjects.emplace(quad.getId(), std::move(quad));

        for (auto &kv : gameObjects)
        {
          auto tex = kv.second.textures;

          auto colorInfo = tex[0]->getDescriptorInfo();
          auto specInfo = tex[1]->getDescriptorInfo();
          auto normInfo = tex[2]->getDescriptorInfo();
          auto dispInfo = tex[3]->getDescriptorInfo();

          LveDescriptorWriter(*matLayout, *globalPool)
                .writeImage(1, &colorInfo) // colour
                .writeImage(2, &specInfo) //spec 
                .writeImage(3, &normInfo) //normal
                .writeImage(4, &dispInfo) //displacement 
                .build(kv.second.descriptorSet);
        }

        std::vector<glm::vec3> lightColors{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f},
            {.0157f, 0.824f, 0.745f}    //0x04d3be
        };

        for (int i = 0; i < lightColors.size(); i++) 
        {
            auto pointLight = LveGameObject::makePointLight(0.2f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(
                glm::mat4(1.f),
                (i * glm::two_pi<float>()) / lightColors.size(),
                { 0.f, -1.f, 0.f });
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
	}

}
