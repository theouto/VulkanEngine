#include "../include/first_app.hpp"

#include "../include/keyboard_movement_controller.hpp"
#include "../include/lve_camera.hpp"
#include "../include/lve_buffer.hpp"
#include "../include/lve_frame_info.hpp"
#include "../include/the_scene.hpp"


#include "../systems/point_light_system.hpp"
#include "../systems/simple_render_system.hpp"
#include "../systems/skybox_system.hpp"
#include "../systems/shadow_system.hpp"
#include "../systems/normal_spec.hpp"
#include "../systems/ambientocclusion_system.hpp"
#include "../systems/depth_buffer.hpp"


#include <GLFW/glfw3.h>
#include <algorithm>
#include <glm/ext/vector_float3.hpp>
#include <memory>
#include <vulkan/vulkan_core.h>

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
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        std::vector<std::unique_ptr<LveBuffer>> shadowBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
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

        auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)            
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
            .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
            .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
            .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++)
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            auto shadowInfo = lveRenderer.getShadowInfo();
            auto depthInfo = lveRenderer.getDepthInfo();
            auto normalSpecInfo = lveRenderer.getNormalInfo();

            LveDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo) 
                .writeImage(1, &shadowInfo)
                .writeImage(2, &depthInfo)
                .writeImage(3, &normalSpecInfo)
                .build(globalDescriptorSets[i]);
        }

        std::vector<VkDescriptorSetLayout> setLayouts = {
            globalSetLayout->getDescriptorSetLayout(),
            matLayout->getDescriptorSetLayout()};

		SimpleRenderSystem simpleRenderSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(), setLayouts};
        PointLightSystem pointLightSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
        SkyboxSystem skybox{lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout(), *globalPool};

        DirectionalLightSystem shadowSystem{lveDevice, lveRenderer.getSwapChainShadowPass(),globalSetLayout->getDescriptorSetLayout()};
        NormalSpecPass normalSpecPass{lveDevice, lveRenderer.getSwapChainNormalPass(), globalSetLayout->getDescriptorSetLayout(), normalLayout->getDescriptorSetLayout()};
        DepthBuffer depthBuffer{lveDevice, lveRenderer.getSwapChainDepthPass(), globalSetLayout->getDescriptorSetLayout()};

        AOSystem AOSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), *globalPool, globalSetLayout->getDescriptorSetLayout()};

        LveCamera camera{};
 
        auto viewerObject = LveGameObject::createGameObject();
        viewerObject.transform.translation.z = -1.5f;
	

	    // https://www.glfw.org/docs/3.3/input_guide.html#raw_mouse_motion <- important
        glfwSetInputMode(lveWindow.getGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported())
        {
            glfwSetInputMode(lveWindow.getGLFWwindow(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }     

        glfwSetInputMode(lveWindow.getGLFWwindow(), GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
        KeyboardMovementController cameraController{};
        cameraController.mousecontrol = true;
        double mouseX = 0.f;
        double mouseY = 0.f;
	
	auto currentTime = std::chrono::high_resolution_clock::now();

    std::cout << "\n\n\nAll loaded, rendering:\n\n";
    float radius = 10.f;
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

            glm::vec3 offset = {-radius, radius, -2.f};
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
                ubo.viewStat = camera.getviewStat();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                glm::vec3 lightPos = {1.f, 2.f, 2.f};
                glm::mat4 projMat = DirectionalLightSystem::lightViewProjection(
                  lightPos, 
                  frameInfo.camera.getPosition() + offset, 
                  radius);

                auto rotate = glm::rotate(glm::mat4(1.f), frameInfo.frameTIme, { 0.1f, -0.5f, 0.f });
                gameObjects.at(3).transform.translation = glm::vec3(rotate 
                              * glm::vec4(gameObjects.at(3).transform.translation, 1.f));

                //render shadowmap
		        lveRenderer.beginShadowRenderPass(commandBuffer);
                shadowSystem.drawDepth(frameInfo, projMat, lightPos);
                lveRenderer.endSwapChainRenderPass(commandBuffer);

                //Depth Prepass
                lveRenderer.beginDepthRenderPass(commandBuffer);
                depthBuffer.renderGameObjects(frameInfo);
                lveRenderer.endSwapChainRenderPass(commandBuffer);

                //Normal and specular pass
                lveRenderer.beginNormalRenderPass(commandBuffer);
                normalSpecPass.drawDepth(frameInfo);
                lveRenderer.endSwapChainRenderPass(commandBuffer);
 
                //render 
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                
                //skybox
                skybox.render(frameInfo);

                //geometry pass excl. skybox
                simpleRenderSystem.renderGameObjects(frameInfo, projMat, lightPos);

                //Ambient Occlusion
                AOSystem.render(frameInfo);

                //renders light dots
                pointLightSystem.render(frameInfo);
                
                lveRenderer.endSwapChainRenderPass(commandBuffer);
				lveRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(lveDevice.device());
	}

	void FirstApp::loadGameObjects()
	{
      /*
        std::vector<std::shared_ptr<LveTextures>> granite = {std::make_unique<LveTextures>( lveDevice, "textures/Granite001A_2K-PNG_Color.png", LveTextures::COLOR ),
        std::make_unique<LveTextures>( lveDevice, "textures/Granite001A_2K-PNG_Roughness.png", LveTextures::SPECULAR ),
        std::make_unique<LveTextures>( lveDevice, "textures/Granite001A_2K-PNG_NormalGL.png", LveTextures::NORMAL ),
        std::make_unique<LveTextures>( lveDevice, "textures/Granite001A_2K-PNG_Displacement.png", LveTextures::DEPTH ),
        std::make_unique<LveTextures>( lveDevice, "textures/NA.png", LveTextures::SPECULAR),
        std::make_unique<LveTextures>(lveDevice, "textures/NAM.png", LveTextures::SPECULAR)
        };
        

       
        
        };
        

        
        std::vector<std::shared_ptr<LveTextures>> sMetal = {std::make_unique<LveTextures>( lveDevice, "textures/Metal051A_2K-PNG_Color.png", LveTextures::COLOR ),
            std::make_unique<LveTextures>( lveDevice, "textures/Metal051A_2K-PNG_Roughness.png", LveTextures::SINGLE_UNORM ),
            std::make_unique<LveTextures>( lveDevice, "textures/Metal051A_2K-PNG_NormalGL.png", LveTextures::NORMAL ),
            std::make_unique<LveTextures>( lveDevice, "textures/Metal051A_2K-PNG_Displacement.png", LveTextures::SINGLE_UNORM ),
            std::make_unique<LveTextures>( lveDevice, "textures/NA.png", LveTextures::SINGLE_UNORM),
            std::make_unique<LveTextures>(lveDevice, "textures/Metal051A_2K-PNG_Metalness.png", LveTextures::SINGLE_UNORM)
        };
        
        
        std::vector<std::shared_ptr<LveTextures>> ice = {std::make_unique<LveTextures>( lveDevice, "textures/Ice003_2K-PNG_Color.png", LveTextures::COLOR ),
            std::make_unique<LveTextures>( lveDevice, "textures/Ice003_2K-PNG_Roughness.png", LveTextures::SINGLE_UNORM ),
            std::make_unique<LveTextures>( lveDevice, "textures/Ice003_2K-PNG_NormalGL.png", LveTextures::NORMAL ),
            std::make_unique<LveTextures>( lveDevice, "textures/Ice003_2K-PNG_Displacement.png", LveTextures::SINGLE_UNORM ),
            std::make_unique<LveTextures>( lveDevice, "textures/NA.png", LveTextures::SINGLE_UNORM),
            std::make_unique<LveTextures>(lveDevice, "textures/NAM.png", LveTextures::SINGLE_UNORM)
        };
        */
       

        matLayout = LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //albedo
            .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //specular
            .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //normal
            .addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //roughness
            .addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //AO
            .addBinding(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //metalness
            .build();

        normalLayout = LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();
        
        /*
        LveMaterials retriever{lveDevice};
        std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "models/smooth_vase.obj");
        auto sVase = LveGameObject::createGameObject();
        sVase.model = lveModel;
        sVase.transform.translation = { -.5f, .2f, 0.f };
        sVase.transform.scale = { 2.f, 2.f, 2.f };
        sVase.textures = retriever.retrieveMaterial("materials/wet_rock.thmat");
        gameObjects.emplace(sVase.getId(), std::move(sVase));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/flat_vase.obj");
        auto vase = LveGameObject::createGameObject();
        vase.model = lveModel;
        vase.transform.translation = { .5f, .2f, 0.f };
        vase.transform.scale = { 1.5f, 1.5f, 1.5f };
        vase.textures = retriever.retrieveMaterial("materials/wet_rock.thmat");
        gameObjects.emplace(vase.getId(), std::move(vase));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/quad.obj");
        auto quad = LveGameObject::createGameObject();
        quad.model = lveModel;
        quad.transform.translation = { 0.f, .5f, 0.f };
        quad.transform.scale = { 6.f, 1.f, 6.f };
        quad.textures = retriever.retrieveMaterial("materials/wet_rock.thmat");;
        gameObjects.emplace(quad.getId(), std::move(quad));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/pleasepot.obj");
        auto pot = LveGameObject::createGameObject();
        pot.model = lveModel;
        pot.transform.translation = {0.f, -0.5f, 2.f};
        pot.transform.scale = {0.3f, -0.3f, 0.3f};
        pot.textures = retriever.retrieveMaterial("materials/wet_rock.thmat");
        gameObjects.emplace(pot.getId(), std::move(pot));

        
        lveModel = LveModel::createModelFromFile(lveDevice, "models/cube.obj");
        auto quad2 = LveGameObject::createGameObject();
        quad2.model = lveModel;
        quad2.transform.translation = {0.f, 0.5f, 2.f};
        quad2.transform.scale = {1.f, -1.f, 1.f};
        quad2.transform.rotation = {0.f, 2.5f, 0.f};
        quad2.textures = retriever.retrieveMaterial("materials/wet_rock.thmat");
        gameObjects.emplace(quad2.getId(), std::move(quad2));
        

        for (auto &kv : gameObjects)
        {
          kv.second.write_material(*matLayout, *normalLayout, *globalPool);
        }
        

        */

        LveScene sceneManager{ lveDevice};

        sceneManager.load("scenes/test_scene.ths", *matLayout, 
                          *normalLayout, *globalPool, gameObjects);
        
        std::vector<glm::vec3> lightColors{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f},
            {.0157f, 0.824f, 0.745f}    //0x04d3be
        };

        auto pointLight = LveGameObject::makePointLight(0.9f);
        pointLight.color = {1.f, 1.f, 1.f};
        pointLight.transform.translation = glm::vec3(0.7f, -0.2f, 0.7f);
        std::cout << pointLight.getId() << "\n\n\n\n";
        gameObjects.emplace(pointLight.getId(), std::move(pointLight));

        for (int i = 0; i < lightColors.size(); i++) 
        {
            pointLight = LveGameObject::makePointLight(0.7f);
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
