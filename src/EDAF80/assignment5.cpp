#include "assignment5.hpp"
#include "parametric_shapes.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/ShaderProgramManager.hpp"

#include <imgui.h>
#include <tinyfiledialogs.h>

#include <stdexcept>
#include <core\node.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <stdexcept>

edaf80::Assignment5::Assignment5(WindowManager& windowManager) :
	mCamera(0.5f * glm::half_pi<float>(),
	        static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	        0.01f, 1000.0f),
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};

	window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 5", window_datum, config::msaa_rate);
	if (window == nullptr) {
		throw std::runtime_error("Failed to get a window: aborting!");
	}
}

void
edaf80::Assignment5::run()
{
	// Set up the camera
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 6.0f));
	mCamera.mMouseSensitivity = 0.003f;
	mCamera.mMovementSpeed = 3.0f; // 3 m/s => 10.8 km/h
	auto camera_position = mCamera.mWorld.GetTranslation();

	// Create the shader programs
	ShaderProgramManager program_manager;
	GLuint fallback_shader = 0u;
	program_manager.CreateAndRegisterProgram("Fallback",
	                                         { { ShaderType::vertex, "EDAF80/fallback.vert" },
	                                           { ShaderType::fragment, "EDAF80/fallback.frag" } },
	                                         fallback_shader);
	if (fallback_shader == 0u) {
		LogError("Failed to load fallback shader");
		return;
	}

	GLuint diffuse_shader = 0u;
	program_manager.CreateAndRegisterProgram("Diffuse",
		{ { ShaderType::vertex, "EDAF80/diffuse.vert" },
		  { ShaderType::fragment, "EDAF80/diffuse.frag" } },
		diffuse_shader);
	if (diffuse_shader == 0u)
		LogError("Failed to load diffuse shader");

	GLuint normal_shader = 0u;
	program_manager.CreateAndRegisterProgram("Normal",
		{ { ShaderType::vertex, "EDAF80/normal.vert" },
			{ ShaderType::fragment, "EDAF80/normal.frag" } },
		normal_shader);
	if (normal_shader == 0u)
		LogError("Failed to load normal shader");

	GLuint texcoord_shader = 0u;
	program_manager.CreateAndRegisterProgram("Texture coords",
		{ { ShaderType::vertex, "EDAF80/texcoord.vert" },
		  { ShaderType::fragment, "EDAF80/texcoord.frag" } },
		texcoord_shader);
	if (texcoord_shader == 0u)
		LogError("Failed to load texcoord shader");

	//
	// Todo: Insert the creation of other shader programs.
	//       (Check how it was done in assignment 3.)
	//
	GLuint skybox_shader = 0u;
	program_manager.CreateAndRegisterProgram("Skybox",
		{ { ShaderType::vertex, "EDAF80/skybox.vert" },
		  { ShaderType::fragment, "EDAF80/skybox.frag" } },
		skybox_shader);
	if (skybox_shader == 0u)
		LogError("Failed to load skybox shader");

	// WAVES:
	GLuint waves_shader = 0u;
	program_manager.CreateAndRegisterProgram("Waves",
		{ { ShaderType::vertex, "EDAF80/waves.vert" },
		  { ShaderType::fragment, "EDAF80/waves.frag" } },
		waves_shader);
	if (waves_shader == 0u)
		LogError("Failed to load phong shader");

	// PHONG SHADER:
	GLuint phong_shader = 0u;
	program_manager.CreateAndRegisterProgram("Phong",
		{ { ShaderType::vertex, "EDAF80/phong.vert" },
		  { ShaderType::fragment, "EDAF80/phong.frag" } },
		phong_shader);
	if (phong_shader == 0u)
		LogError("Failed to load phong shader");

	//
	// Todo: Load your geometry
	//
	auto quad_shape = parametric_shapes::createQuad(500.0f, 500.0f, 50u, 50u);
	if (quad_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the demo sphere");
		return;
	}
	auto skybox_shape = parametric_shapes::createSphere(500.0f, 100u, 100u);
	if (skybox_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the skybox");
		return;
	}
	auto sphere_shape = parametric_shapes::createSphere(5.0f, 100u, 100u);
	if (skybox_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the skybox");
		return;
	}

	float ellapsed_time_s = 0.0f;
	auto const waves_set_uniforms = [&ellapsed_time_s, &camera_position](GLuint program) {
		glUniform1f(glGetUniformLocation(program, "time"), ellapsed_time_s);
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
	};


	auto my_cube_map_id = bonobo::loadTextureCubeMap("C:/Skola/datorgrafik/res/cubemaps/NissiBeach2/posx.jpg", "C:/Skola/datorgrafik/res/cubemaps/NissiBeach2/negx.jpg",
		"C:/Skola/datorgrafik/res/cubemaps/NissiBeach2/posy.jpg", "C:/Skola/datorgrafik/res/cubemaps/NissiBeach2/negy.jpg",
		"C:/Skola/datorgrafik/res/cubemaps/NissiBeach2/posz.jpg", "C:/Skola/datorgrafik/res/cubemaps/NissiBeach2/negz.jpg");
	Node skybox;
	skybox.set_geometry(skybox_shape);
	skybox.set_program(&skybox_shader, waves_set_uniforms);
	skybox.add_texture("my_skybox", my_cube_map_id, GL_TEXTURE_CUBE_MAP);

	auto my_waves_id = bonobo::loadTexture2D("C:/Skola/datorgrafik/res/textures/waves.png");
	Node water;
	water.set_geometry(quad_shape);
	water.set_program(&waves_shader, waves_set_uniforms);
	water.add_texture("my_skybox", my_cube_map_id, GL_TEXTURE_CUBE_MAP);
	water.add_texture("my_waves", my_waves_id, GL_TEXTURE_2D);

	Node player_sphere;
	player_sphere.set_geometry(sphere_shape);
	player_sphere.set_program(&phong_shader, waves_set_uniforms);

	// Follow sphere?
	camera_position = player_sphere.get_transform().GetTranslation();

	glClearDepthf(1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	// Enable face culling to improve performance:
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glCullFace(GL_BACK);


	auto lastTime = std::chrono::high_resolution_clock::now();

	auto polygon_mode = bonobo::polygon_mode_t::fill;
	bool show_logs = true;
	bool show_gui = true;
	bool shader_reload_failed = false;

	glm::vec3 player_sphere_position = player_sphere.get_transform().GetTranslation();
	glm::vec3 player_previous_position = player_sphere_position;
	glm::mat3 player_sphere_rotation = player_sphere.get_transform().GetRotation();

	while (!glfwWindowShouldClose(window)) {
		auto const nowTime = std::chrono::high_resolution_clock::now();
		auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
		lastTime = nowTime;
		ellapsed_time_s += std::chrono::duration<float>(deltaTimeUs).count();

		// Calculate angle between
		//glm::vec3 lookAtVector = player_previous_position - player_sphere_position;
		//float value = ((player_previous_position.x - 10) / lookAtVector.z);
		//float angle = atan(value);
	

		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();
		mCamera.Update(deltaTimeUs, inputHandler);
		

		if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
			shader_reload_failed = !program_manager.ReloadAllPrograms();
			if (shader_reload_failed)
				tinyfd_notifyPopup("Shader Program Reload Error",
				                   "An error occurred while reloading shader programs; see the logs for details.\n"
				                   "Rendering is suspended until the issue is solved. Once fixed, just reload the shaders again.",
				                   "error");
		}
		if (inputHandler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
			show_logs = !show_logs;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
			show_gui = !show_gui;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F11) & JUST_RELEASED)
			mWindowManager.ToggleFullscreenStatusForWindow(window);


		// Retrieve the actual framebuffer size: for HiDPI monitors,
		// you might end up with a framebuffer larger than what you
		// actually asked for. For example, if you ask for a 1920x1080
		// framebuffer, you might get a 3840x2160 one instead.
		// Also it might change as the user drags the window between
		// monitors with different DPIs, or if the fullscreen status is
		// being toggled.
		int framebuffer_width, framebuffer_height;
		glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
		glViewport(0, 0, framebuffer_width, framebuffer_height);


		//
		// Todo: If you need to handle inputs, you can do it here
		//
		if (JUST_RELEASED) {

		}
		if (inputHandler.GetKeycodeState(GLFW_KEY_W) & JUST_PRESSED) {
			player_sphere.get_transform().SetTranslate(glm::vec3(player_previous_position.x + 10, player_previous_position.y, player_previous_position.z));
		}
		if (inputHandler.GetKeycodeState(GLFW_KEY_S) & JUST_PRESSED) {
			player_sphere.get_transform().SetTranslate(glm::vec3(player_previous_position.x - 10, player_previous_position.y, player_previous_position.z));
		}
		if (inputHandler.GetKeycodeState(GLFW_KEY_A) & JUST_PRESSED) {
			player_sphere.get_transform().SetTranslate(glm::vec3(player_previous_position.x, player_previous_position.y, player_previous_position.z - 10));
		}
		if (inputHandler.GetKeycodeState(GLFW_KEY_D) & JUST_PRESSED) {
			player_sphere.get_transform().SetTranslate(glm::vec3(player_previous_position.x, player_previous_position.y, player_previous_position.z + 10));
		}


		// Move player
		
		// Camera follow player
		player_sphere_position = player_sphere.get_transform().GetTranslation();
		mCamera.mWorld.SetTranslate(glm::vec3(player_sphere_position.x - 20, player_sphere_position.y, player_sphere_position.z));
		player_sphere_rotation = player_sphere.get_transform().GetRotation();

		// Camera looks at the player
		mCamera.mWorld.LookAt(player_sphere_position);

		// Set the current position to the player_previous_position after moving and rotating camera
		player_previous_position = player_sphere_position;


		mWindowManager.NewImGuiFrame();

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		bonobo::changePolygonMode(polygon_mode);

		if (!shader_reload_failed) {
			//
			// Todo: Render all your geometry here.
			//
			water.render(mCamera.GetWorldToClipMatrix());
			skybox.render(mCamera.GetWorldToClipMatrix());
			player_sphere.render(mCamera.GetWorldToClipMatrix());
		}


		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//
		// Todo: If you want a custom ImGUI window, you can set it up
		//       here
		//

		if (show_logs)
			Log::View::Render();
		if (show_gui)
			mWindowManager.RenderImGuiFrame();

		glfwSwapBuffers(window);
	}
}

int main()
{
	Bonobo framework;

	try {
		edaf80::Assignment5 assignment5(framework.GetWindowManager());
		assignment5.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}
