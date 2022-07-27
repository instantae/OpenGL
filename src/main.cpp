// OpenGL functionality
#include <glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cstdlib>
#include <vector>
#include <chrono>

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_glfw.h"
#include "vendor/imgui/imgui_impl_opengl3.h"

#include "renderer.h"

#include "VertexBufferLayout.h"
#include "Texture.h"

float deltaTime = 0, lastFrame = 0;

long ssboSize = 1000000;

glm::vec3 cameraPos(55.0f, 25.0f, 30.0f);
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);


void processInput(GLFWwindow* window, ImGuiIO& io); // forward declare

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

struct SSBOIDs
{
	GLuint matrixBuffer;
	GLuint colorsBuffer;
};

struct SSBOArrays
{
	std::vector<glm::mat4> MatrixArray;
	std::vector<glm::vec3> ColorsArray;
};

struct Cubes
{
	Cubes()
	{
	}

	Cubes(glm::vec3 pos, glm::vec3 scale, glm::vec3 rot, glm::vec3 rgba) : position(pos), scale(scale), rotation(rot), color(rgba)
	{
		calcMatrix();
	}

	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::vec3 scale = glm::vec3(1, 1, 1);
	glm::vec3 rotation = glm::vec3(0, 0, 0);
	glm::vec3 color = glm::vec3(1, 1, 1);

	glm::mat4 modelMatrix = glm::mat4(1.0f);


	glm::vec3 const GetPosition() { return position; }

	void calcMatrix()
	{
		glm::mat4 mat = glm::translate(glm::mat4(1.0f), position);
		mat = glm::rotate(mat, rotation.x, glm::vec3(1.0, 0.0, 0.0));
		mat = glm::rotate(mat, rotation.y, glm::vec3(0.0, 1.0, 0.0));
		mat = glm::rotate(mat, rotation.z, glm::vec3(0.0, 0.0, 1.0));
		mat = glm::scale(mat, scale);

		this->modelMatrix = mat;
	}
};


void AddCube(std::vector<Cubes>& world, SSBOArrays& ssbo, Cubes obj);
void UpdateInstanceBuffer(SSBOIDs bufferIDs, SSBOArrays bufferArrays);
void RotateAround2D(glm::vec2 inPos, float inRadius, float inAngle, glm::vec2& outPos);


int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	const char* glsl_version("#version 460");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);

	int width_mm, height_mm;
	glfwGetMonitorPhysicalSize(monitor, &width_mm, &height_mm);

	float xscale, yscale;
	glfwGetMonitorContentScale(monitor, &xscale, &yscale);

	std::cout << "Monitor: " << vidmode->width << "x" << vidmode->height << " @" << vidmode->refreshRate << "Hz" << std::endl;
	std::cout << "scale: " << xscale << "x" << yscale << std::endl;


	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(1280, 720, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	int xpos, ypos;
	glfwGetWindowPos(window, &xpos, &ypos);
	std::cout << "window pos: " << xpos << "x" << ypos << std::endl;

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSwapInterval(0); // VSYNC

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "failed to initialize glad" << std::endl;
		return -1;
	}
	else
		std::cout << glGetString(GL_VERSION) << std::endl;

	// 2D square vertices (needs indices)
	float pos[] = {
		-0.5f, -0.5f, 0.0f, 0.0f,
		 0.5f, -0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f, 1.0f, 1.0f,
		-0.5f,  0.5f, 0.0f, 1.0f
	};

	GLuint indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	float cubePos[] = {
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};


	GLCall(glEnable(GL_BLEND));
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GLCall(glEnable(GL_DEPTH_TEST));

	/*VertexArray vao;
	VertexBuffer vbo(cubePos, 36 * sizeof(float));

	VertexBufferLayout layout;
	layout.Push<GLfloat>(3);
	layout.Push<GLfloat>(2);
	vao.AddBuffer(vbo, layout);

	IndexBuffer ibo(indices, 6);*/

	// CTRL+K+C to comment selection
	// CTRL+K+U to uncomment selection

	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubePos), cubePos, GL_STATIC_DRAW);

	//position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	//normals attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	Shader shader("res/shaders/basic.shader");
	shader.Bind();

	Texture texture("res/textures/blanksquare.png");
	texture.Bind();
	shader.SetUniform1i("u_Texture", 0);

	/*vao.Unbind();
	vbo.Unbind();
	ibo.Unbind();*/
	shader.Unbind();


	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
	glm::vec3 lightPos(47.0f, 47.0f, 5.0f);


	Shader instanceShader("res/shaders/instanced.shader");
	instanceShader.Bind();
	instanceShader.SetUniform3f("u_LightColor", lightColor);
	instanceShader.SetUniform3f("u_lightpos", lightPos);
	instanceShader.Unbind();

	Shader lightsourceShader("res/shaders/lightsource.shader");


	lightsourceShader.Bind();
	lightsourceShader.SetUniform3f("u_LightColor", lightColor);
	lightsourceShader.Unbind();


	glm::vec3 viewTrans(0.0f, 0.0f, 0.0f);

	//glm::mat4 projectionMatrix = glm::ortho(0.0f, 16.0f, 0.0f, 9.0f, -1.0f, 1.0f);
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 200.0f);
	glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), viewTrans);


	std::vector<Cubes> World;
	SSBOArrays SSBO;


	/*{
		Cubes plane(glm::vec3(49.0f, 49.0f, 0.0f), glm::vec3(100.0f, 100.0f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

		World.push_back(plane);
		SSBO.MatrixArray.push_back(plane.modelMatrix);
		SSBO.ColorsArray.push_back(plane.color);
	}*/
	

	Renderer renderer;

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	float increment = 0.05f;

	bool isWireframe = false;

	// ImGui stuff
	bool show_demo_window = true;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	float pitch = -35.0f, yaw = -90.0f, fov = 45.0f;

	int input = 0;

	SSBOIDs BufferIDs;

	// SSBO - Matrices
	glGenBuffers(1, &BufferIDs.matrixBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, BufferIDs.matrixBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, ssboSize * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, BufferIDs.matrixBuffer);
	
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, SSBO.MatrixArray.size() * sizeof(glm::mat4), SSBO.MatrixArray.data());
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// SSBO - Colors
	glGenBuffers(1, &BufferIDs.colorsBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, BufferIDs.colorsBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, ssboSize * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, BufferIDs.colorsBuffer);

	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, SSBO.ColorsArray.size() * sizeof(glm::mat4), SSBO.ColorsArray.data());


	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	// UBO stuff
	GLuint uboMatrices;
	glGenBuffers(1, &uboMatrices);

	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);

	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

	projectionMatrix = glm::perspective(glm::radians(fov), 1280.0f / 720.0f, 0.1f, 200.0f);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projectionMatrix));

	viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(viewMatrix));
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboMatrices);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	srand(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	
	
		glm::vec3 boxPos(47.0f, 47.0f, 2.0f);
	{
		AddCube(World, SSBO, Cubes(boxPos, glm::vec3(3.0), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0, 0.0, 0.37)));
	}

	Cubes light(lightPos, glm::vec3(0.5), glm::vec3(1.0), lightColor);
	light.calcMatrix();

	UpdateInstanceBuffer(BufferIDs, SSBO);

	float angle = 0.0f, radius = 10.0f, speed = 1.0f;

	bool rotateAroundXY = true, rotateAroundXZ = false, rotateAroundYZ = false, paused = false, reverse = false;

	glm::vec3 savedPosition;

	float specularStrength = 0.5, specularShininess = 32;


	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		renderer.Clear();

		ImGuiIO& io = ImGui::GetIO();

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window, io);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		/*yaw += io.MouseDelta.x;
		pitch += io.MouseDelta.y;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;*/

		/*glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(direction);*/


		fov -= 2 * io.MouseWheel;
		if (fov < 1.0f)
			fov = 1.0f;
		if (fov > 90.0f)
			fov = 90.0f;


		glm::vec3 direction = cameraFront;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = -sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(direction);

		projectionMatrix = glm::perspective(glm::radians(fov), 1280.0f / 720.0f, 0.1f, 200.0f);
		glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projectionMatrix));

		viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(viewMatrix));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindVertexArray(VAO);
		//gradually orbit light around y axis
		
		

		glm::vec3 pos;
		glm::vec2 newPos;
		glm::vec2 rotateAround;
		
		// light box rotation behavior
		{
			if (angle > 360.0f)
				angle = 0.0f;
			if (angle < 0.0f)
				angle = 360.0f;

			if (paused)
			{
				pos = savedPosition;
			}
			if (!paused && !reverse)
			{
				float step = 1 * deltaTime;
				angle += step;

				angle += speed;
			}
			if (!paused && reverse)
			{
				float step = -1 * deltaTime;
				angle += step;

				angle -= speed;
			}
			if (rotateAroundXY)
			{
				pos = lightPos;
				newPos = glm::vec2(pos.x, pos.y);
				rotateAround = glm::vec2(boxPos.x, boxPos.y);

				RotateAround2D(rotateAround, radius, glm::radians(angle), newPos);
				pos.x = newPos.x;
				pos.y = newPos.y;
			}
			if (rotateAroundXZ)
			{
				pos = lightPos;
				newPos = glm::vec2(pos.x, pos.z);
				rotateAround = glm::vec2(boxPos.x, boxPos.z);

				RotateAround2D(rotateAround, radius, glm::radians(angle), newPos);
				pos.x = newPos.x;
				pos.z = newPos.y;
			}
			if (rotateAroundYZ)
			{
				pos = lightPos;
				newPos = glm::vec2(pos.y + radius, pos.z);
				rotateAround = glm::vec2(boxPos.y, boxPos.z);

				RotateAround2D(rotateAround, radius, glm::radians(angle), newPos);
				pos.y = newPos.x;
				pos.z = newPos.y;
			}
		}

		lightsourceShader.Bind();
		glm::mat4 modelMatrix = translate(glm::mat4(1.0f), pos);
		light.position = pos;

		lightsourceShader.SetUniformMat4f("u_ModelMatrix", modelMatrix);
		lightsourceShader.SetUniformMat4f("u_ViewMatrix", viewMatrix);
		lightsourceShader.SetUniformMat4f("u_ProjectionMatrix", projectionMatrix);
		
			
		glDrawArrays(GL_TRIANGLES, 0, 36);
		lightsourceShader.Unbind();
		glBindVertexArray(0);


		glBindVertexArray(VAO);

		instanceShader.Bind();
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, BufferIDs.colorsBuffer);
		instanceShader.SetUniform3f("u_lightpos", light.GetPosition());
		instanceShader.SetUniform3f("u_viewpos", cameraPos);
		instanceShader.SetUniform1f("u_specularstrength", specularStrength);
		instanceShader.SetUniform1f("u_specularshininess", specularShininess);

		UpdateInstanceBuffer(BufferIDs, SSBO);


		glDrawArraysInstanced(GL_TRIANGLES, 0, 36, SSBO.MatrixArray.size());
		instanceShader.Unbind();
		glBindVertexArray(0);

		//{
		//	glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), translationA);
		//	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotA), glm::vec3(0.0, 1.0, 0.0));

		//	viewMatrix = glm::scale(viewMatrix, glm::vec3(scale));

		//	glm::mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;

		//	shader.Bind();
		//	shader.SetUniform4f("u_Color", u_Color);
		//	shader.SetUniformMat4f("u_MVP", mvp);
		//	//renderer.Draw(vao, ibo, shader);

		//	glDrawArrays(GL_TRIANGLES, 0, 36);
		//}


		//ImGui::ShowDemoWindow(&show_demo_window);

		{
			ImGui::Begin(" ");

			ImGui::SliderFloat3("cameraPosition", &cameraPos.x, -100.0f, 200.0f);
			ImGui::SliderFloat("Pitch", &pitch, -89.0f, 89.0f);
			ImGui::SliderFloat("Yaw", &yaw, -180.0f, 180.0f);


			ImGui::SliderFloat3("lightpos", &pos.x, -100.0f, 100.0f);
			light.position = pos;

			ImGui::SliderFloat2("Rotate around", &rotateAround.x, -100.0f, 100.0f);
			
			if (ImGui::Button("Rotate around X/Y axis"))
			{
				rotateAroundXY = true;
				rotateAroundXZ = false;
				rotateAroundYZ = false;
			}
			if (ImGui::Button("Rotate around X/Z axis"))
			{
				rotateAroundXY = false;
				rotateAroundXZ = true;
				rotateAroundYZ = false;
			}
			if (ImGui::Button("Rotate around Y/Z axis"))
			{
				rotateAroundXY = false;
				rotateAroundXZ = false;
				rotateAroundYZ = true;
			}
			ImGui::Checkbox("Freeze in place", &paused);
				savedPosition = pos;
			ImGui::Checkbox("Reverse rotation", &reverse);

			ImGui::SliderFloat("Angle", &angle, 0.0f, 360.0f);
			ImGui::SliderFloat("Radius", &radius, 0.0f, 20.0f);
			ImGui::SliderFloat("Speed", &speed, 0.001f, 20.0f);


			ImGui::SliderFloat("Specular Strength", &specularStrength, 0.001f, 1.0f);
			ImGui::SliderFloat("Object Shininess", &specularShininess, 0, 256);
			/*ImGui::Text("cameraPos.x = %f, cameraPos.y = %f, cameraPos.z = %f", cameraPos.x, cameraPos.y, cameraPos.z);

			ImGui::Text("Pitch: %f Yaw: %f", pitch, yaw);
			ImGui::Text("Direction.x = %f, Direction.y = %f, Direction.z = %f", direction.x, direction.y, direction.z);*/
			ImGui::Text("FOV: %f", fov);

			ImGui::Text("Number of cubes in world: %i", World.size());

			if (ImGui::Button("Add Cube"))
			{
				AddCube(World, SSBO, Cubes(glm::vec3(rand() % 99, rand() % 99, 1.0f), glm::vec3(1.0, 1.0, 1.0), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 0.0, 1.0)));
				UpdateInstanceBuffer(BufferIDs, SSBO);
			}


			ImGui::InputInt("Draw X cubes", &input);
			if (ImGui::Button("Draw"))
			{
				for (int i = 0; i <= input; i++)
				{
					AddCube(World, SSBO, Cubes(glm::vec3(rand() % 99, rand() % 99, 1.0f), glm::vec3(1.0, 1.0, 1.0), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 0.0, 1.0)));
				}
				UpdateInstanceBuffer(BufferIDs, SSBO);
			}

			ImGui::Checkbox("Wireframe mode", &isWireframe);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
		

		/*{
			ImGui::Begin("world object tracker");

			for (int i = 0; i < World.size(); i++)
			{
				ImGui::Text("Cube %i at x=%f, y=%f, z=%f", i, World[i].position.x, World[i].position.y, World[i].position.z);
				ImGui::Text("\t rotation: x=%f, y=%f, z=%f", World[i].rotation.x, World[i].rotation.y, World[i].rotation.z);
				ImGui::Text("\t scale: x=%f, y=%f, z=%f", World[i].scale.x, World[i].scale.y, World[i].scale.z);
			}

			ImGui::End();
		}*/

		if (isWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwDestroyWindow(window);


	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();


	return 0;
}

void processInput(GLFWwindow* window, ImGuiIO& io)
{
	float cameraSpeed = 10.0f * deltaTime;

	if (ImGui::IsKeyDown(ImGuiKey_Escape))
		glfwSetWindowShouldClose(window, true);

	/*if (ImGui::IsKeyDown(ImGuiKey_W))
		cameraPos += cameraSpeed * cameraFront;

	if (ImGui::IsKeyDown(ImGuiKey_S))
		cameraPos -= cameraSpeed * cameraFront;

	if (ImGui::IsKeyDown(ImGuiKey_A))
		cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));

	if (ImGui::IsKeyDown(ImGuiKey_D))
		cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));*/
}


void AddCube(std::vector<Cubes>& world, SSBOArrays& ssbo, Cubes obj)
{
	world.push_back(obj);
	ssbo.MatrixArray.push_back(obj.modelMatrix);
	ssbo.ColorsArray.push_back(obj.color);
}

void UpdateInstanceBuffer(SSBOIDs bufferIDs, SSBOArrays bufferArrays)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferIDs.matrixBuffer);

	if (bufferArrays.MatrixArray.size() == ssboSize)
	{
		ssboSize += ssboSize; // double ssbo size
		glBufferData(GL_SHADER_STORAGE_BUFFER, ssboSize * sizeof(glm::mat4), bufferArrays.MatrixArray.data(), GL_DYNAMIC_DRAW);
		std::cout << "resized model matrix ssbo!" << std::endl;
	}
	else
	{
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, bufferArrays.MatrixArray.size() * sizeof(glm::mat4), bufferArrays.MatrixArray.data());
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferIDs.colorsBuffer);

	if (bufferArrays.ColorsArray.size() == ssboSize)
	{
		ssboSize += ssboSize; // double ssbo size
		glBufferData(GL_SHADER_STORAGE_BUFFER, ssboSize * sizeof(glm::mat4), bufferArrays.ColorsArray.data(), GL_DYNAMIC_DRAW);
		std::cout << "resized model matrix ssbo!" << std::endl;
	}
	else
	{
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, bufferArrays.ColorsArray.size() * sizeof(glm::vec3), bufferArrays.ColorsArray.data());
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void RotateAround2D(glm::vec2 inPos, float inRadius, float inAngle, glm::vec2& outPos)
{
	outPos.x = ((cos(inAngle) * inRadius) + inPos.x);
	outPos.y = ((sin(inAngle) * inRadius) + inPos.y);
}