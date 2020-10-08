#include "assignment4.hpp"
#include "parametric_shapes.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/node.hpp"
#include "core/ShaderProgramManager.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <tinyfiledialogs.h>

#include <stdexcept>

edaf80::Assignment4::Assignment4(WindowManager& windowManager) :
	mCamera(0.5f * glm::half_pi<float>(),
	        static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	        0.01f, 1000.0f),
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};

	window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 4", window_datum, config::msaa_rate);
	if (window == nullptr) {
		throw std::runtime_error("Failed to get a window: aborting!");
	}
}

void
edaf80::Assignment4::run()
{
	// Set up the camera
	mCamera.mWorld.SetTranslate(glm::vec3(25.0f, 7.0f, 50.0f));
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
	// SKYBOX SHADER:
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
		LogError("Failed to load waves shader");

	//
	// Todo: Load your geometry
	//
	auto quad_shape = parametric_shapes::createQuad(50.0f, 50.0f, 50u, 50u);
	if (quad_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the demo sphere");
		return;
	}
	auto skybox_shape = parametric_shapes::createSphere(100.0f, 100u, 100u);
	if (skybox_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the skybox");
		return;
	}
	auto sphere_shape = parametric_shapes::createSphere(10.0f, 100u, 100u);
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

	Node sphere_water;
	sphere_water.set_geometry(sphere_shape);
	sphere_water.set_program(&waves_shader, waves_set_uniforms);
	sphere_water.add_texture("my_skybox", my_cube_map_id, GL_TEXTURE_CUBE_MAP);
	sphere_water.add_texture("my_waves", my_waves_id, GL_TEXTURE_2D);

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

	while (!glfwWindowShouldClose(window)) {
		auto const nowTime = std::chrono::high_resolution_clock::now();
		auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
		lastTime = nowTime;
		ellapsed_time_s += std::chrono::duration<float>(deltaTimeUs).count();

		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();
		mCamera.Update(deltaTimeUs, inputHandler);
		camera_position = mCamera.mWorld.GetTranslation();

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


		mWindowManager.NewImGuiFrame();

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		bonobo::changePolygonMode(polygon_mode);


		if (!shader_reload_failed) {
			//
			// Todo: Render all your geometry here.
			//
			//demo_sphere.render(mCamera.GetWorldToClipMatrix());
			water.render(mCamera.GetWorldToClipMatrix());
			skybox.render(mCamera.GetWorldToClipMatrix());
			sphere_water.render(mCamera.GetWorldToClipMatrix());
		}


		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//
		// Todo: If you want a custom ImGUI window, you can set it up
		//       here
		//


		bool opened = ImGui::Begin("Scene Control", nullptr, ImGuiWindowFlags_None);
		if (opened) {
			bonobo::uiSelectPolygonMode("Polygon mode", polygon_mode);
		}
		ImGui::End();

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
		edaf80::Assignment4 assignment4(framework.GetWindowManager());
		assignment4.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}
