#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Shader.h"
#include "Camera.h"
#include "Terrain.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
 
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 40.0f, 80.0f));
bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f; 
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "EvoSim", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	Terrain terrain(128, 128, 0.05f, 10.0f, 6);

	Shader cameraShader("shaders/camera.vert", "shaders/camera.frag");
	Shader terrainShader("shaders/terrain.vert", "shaders/terrain.frag");

	terrain.LoadTextures(
		"resources/textures/water_deep.png",
		"resources/textures/water_shallow.png",
		"resources/textures/sand.png",
		"resources/textures/grass.png",
		"resources/textures/forest.png",
		"resources/textures/rock.png",
		"resources/textures/snow.png"
	);

	terrainShader.use();
	terrainShader.setVec3("lightDir", glm::normalize(glm::vec3(0.4f, 1.0f, 0.3f)));
	terrainShader.setVec3("lightColor", glm::vec3(1.0f, 0.95f, 0.8f));

	terrainShader.setFloat("heightDeepWater", -12.0f);
	terrainShader.setFloat("heightShallow", -4.0f);
	terrainShader.setFloat("heightSand", -1.0f);
	terrainShader.setFloat("heightGrass", 2.0f);
	terrainShader.setFloat("heightForest", 10.0f);
	terrainShader.setFloat("heightRock", 15.0f);
	terrainShader.setFloat("heightSnow", 19.0f);

	terrainShader.setFloat("fogStart", 60.0f);
	terrainShader.setFloat("fogEnd", 120.0f);
	terrainShader.setVec3("fogColor", glm::vec3(0.2f, 0.3f, 0.3f));
	terrainShader.setFloat("texScale", 0.05f);

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		//glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
		cameraShader.setMat4("projection", projection);
		glm::mat4 view = camera.GetViewMatrix();
		cameraShader.setMat4("view", view);

		terrainShader.use();
		terrainShader.setVec3("viewPos", camera.Position);
		terrainShader.setMat4("projection", projection);
		terrainShader.setMat4("view", view);
		terrain.Draw(terrainShader);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera.ProcessKeyboard(DOWN, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	camera.ProcessMouseMovement(xpos - lastX, lastY - ypos);
	lastX = xpos;
	lastY = ypos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}