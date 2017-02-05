#include <iostream>
#include <windows.h>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// GLM Mathematics (column-major matrices)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "arcball.h"
#include "cameraClass.h"
#include "meshClass.h"
#include "objectShaderProgram.h"
#include "kinectDataClass.h"

const GLfloat HIGHESTFRAMERATE = 30;
GLint WIDTH = 1000, HEIGHT = 800;

glm::vec3 lightPos(-1.9f, 0.6f, 4.0f);

Camera camera(glm::vec3(0.0f, 1.0f, 10.0f));

bool keys[1024];
// Time between current frame and last frame
GLfloat deltaTime = 0.0f, lastFrameTime = 0.0f;

ArcBall arcBall((GLfloat)WIDTH, (GLfloat)HEIGHT);
glm::mat4 savedModelMatrix = glm::mat4();
bool isLeftDown, isMiddleDown, isRightDown;
glm::vec2 curMousePos;

void CursorPosFunc(GLFWwindow*, double, double);
void KeyFunc(GLFWwindow*, int, int, int, int);
void DoMovement();
void MouseButtonFunc(GLFWwindow*, int, int, int);
void ScrollFunc(GLFWwindow*, double, double);
void WindowSizeFunc(GLFWwindow*, int, int);

int main()
{
#pragma region InitGLFW
	if (!glfwInit()){
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "SkeletalAnimation", nullptr, nullptr);
	if (window == nullptr){
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetWindowPos(window, 720, 50);
	glfwMakeContextCurrent(window);

	glfwSetCursorPosCallback(window, CursorPosFunc);
	glfwSetKeyCallback(window, KeyFunc);
	glfwSetMouseButtonCallback(window, MouseButtonFunc);
	glfwSetScrollCallback(window, ScrollFunc);
	glfwSetWindowSizeCallback(window, WindowSizeFunc);
#pragma endregion

#pragma region InitGLEW
	// Init GLEW BEFORE calling any opengl functions
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK){
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}
#pragma endregion

	// =========== =========== =========== ===========
	// Begin Using OpenGL
	glViewport(0, 0, WIDTH, HEIGHT);

	ShaderProgram shader("../../model.vs", "../../model.fs");
	ShaderProgram shader1("shader.vs", "shader.fs");
	// Mesh fileModel("E:/hammerhead.dae"); // will not scale the model into [-1, 1];
	Mesh fileModel("E:/ironMan.dae"); // will not scale the model into [-1, 1];
	KinectData kinecData;
	kinecData.Init();
	
	shader.Use();
	//glUniform3f(glGetUniformLocation(shader.shaderProgram, "Material1.ambient"), 1.0f, 0.5f, 0.31f);
	//glUniform3f(glGetUniformLocation(shader.shaderProgram, "Material1.diffuse"), 1.0f, 0.5f, 0.31f);
	//glUniform3f(glGetUniformLocation(shader.shaderProgram, "Material1.specular"), 0.5f, 0.5f, 0.5f);
	//glUniform1f(glGetUniformLocation(shader.shaderProgram, "Material1.shininess"), 32.0f);
	//glUniform3f(glGetUniformLocation(shader.shaderProgram, "Light1.position"), lightPos.x, lightPos.y, lightPos.z);
	//glUniform3f(glGetUniformLocation(shader.shaderProgram, "Light1.ambient"), 0.2f, 0.2f, 0.2f);
	//glUniform3f(glGetUniformLocation(shader.shaderProgram, "Light1.diffuse"), 0.5f, 0.5f, 0.5f);
	//glUniform3f(glGetUniformLocation(shader.shaderProgram, "Light1.specular"), 1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(shader.shaderProgram, "Material1.ambient"), 0.3f, 0.3f, 0.3f);
	glUniform3f(glGetUniformLocation(shader.shaderProgram, "Material1.diffuse"), 0.8f, 0.8f, 0.8f);
	glUniform3f(glGetUniformLocation(shader.shaderProgram, "Material1.specular"), 0.2f, 0.2f, 0.2f);
	glUniform1f(glGetUniformLocation(shader.shaderProgram, "Material1.shininess"), 32.0f);
	glUniform3f(glGetUniformLocation(shader.shaderProgram, "Light1.position"), lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(glGetUniformLocation(shader.shaderProgram, "Light1.ambient"), 1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(shader.shaderProgram, "Light1.diffuse"), 1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(shader.shaderProgram, "Light1.specular"), 1.0f, 1.0f, 1.0f);

	while (!glfwWindowShouldClose(window))
	{
		GLfloat curFrameTime = (GLfloat)glfwGetTime();
		deltaTime = curFrameTime - lastFrameTime;
		lastFrameTime = curFrameTime;
		
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		DoMovement();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.Use();

		// get the kinect data
		kinecData.Update();

		// Update the skeleton position according to the hierarchy
		fileModel.UpdateSkeletonUsingKinect(kinecData.joints, kinecData.curTrackingState);

		glUniform3f(glGetUniformLocation(shader.shaderProgram, "ViewPos"), camera.position.x, camera.position.y, camera.position.z);
		
		glm::mat4 model;
		model = arcBall.GetMatrix();
		model *= savedModelMatrix;

		glm::mat4 normalMatrix = glm::transpose(glm::inverse(model));

		glm::mat4 view;
		view = camera.GetViewMatrix();

		glm::mat4 projection;
		projection = glm::perspective((GLfloat)glm::radians(camera.zoomValue), (GLfloat)WIDTH / HEIGHT, 0.1f, 100.0f);

		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "NormalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "Model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "View"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "Projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glEnable(GL_DEPTH_TEST);
		// Draw the loaded model
		fileModel.Draw(shader);
		glDisable(GL_DEPTH_TEST);

		shader1.Use();
		glUniformMatrix4fv(glGetUniformLocation(shader1.shaderProgram, "MVP"), 1, GL_FALSE, glm::value_ptr(projection * view * model));
		fileModel.setAndDraw(shader1, kinecData.joints, kinecData.curTrackingState);

		// Swap the buffers
		glfwSwapBuffers(window);

		double timeElapse = glfwGetTime() - curFrameTime;
		if (timeElapse < 1.0 / HIGHESTFRAMERATE)
			Sleep(static_cast<DWORD>(1000.0 / HIGHESTFRAMERATE - timeElapse * 1000));

	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}

void CursorPosFunc(GLFWwindow* window, double xPos, double yPos)
{
	curMousePos.x = (GLfloat)xPos;
	curMousePos.y = (GLfloat)yPos;

	if (isRightDown || isMiddleDown)
		arcBall.Drag(curMousePos);

	if (isLeftDown)
	{

	}
}

void KeyFunc(GLFWwindow* window, int key, int scanCode, int action, int mode)
{
	// Press ESC to close the application
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

void DoMovement()
{
	// Camera controls
	GLfloat cameraSpeed = 5.0f * deltaTime;
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void MouseButtonFunc(GLFWwindow* window, int button, int action, int mode)
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	curMousePos.x = (GLfloat)xpos;
	curMousePos.y = (GLfloat)ypos;

	if (action == GLFW_PRESS)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			isLeftDown = true;
		}
		if (button == GLFW_MOUSE_BUTTON_MIDDLE)
		{
			isMiddleDown = true;
			arcBall.Click(curMousePos, ArcBall::Pan);
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			isRightDown = true;
			arcBall.Click(curMousePos, ArcBall::Rotation);
		}
	}
	else if (action == GLFW_RELEASE)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
			isLeftDown = false;
		if (button == GLFW_MOUSE_BUTTON_MIDDLE)
			isMiddleDown = false;
		if (button == GLFW_MOUSE_BUTTON_RIGHT)
			isRightDown = false;
		savedModelMatrix = arcBall.GetMatrix() * savedModelMatrix;
		arcBall.End();
	}
}

void ScrollFunc(GLFWwindow* window, double sOffset, double yOffset)
{
	arcBall.Click(glm::vec2(0, 0), ArcBall::Scale);
	arcBall.Drag(glm::vec2(0, -yOffset));
	savedModelMatrix = arcBall.GetMatrix() * savedModelMatrix;
	arcBall.End();

	//camera.ProcessMouseScroll((GLfloat)yOffset);
}

void WindowSizeFunc(GLFWwindow* window, int width, int height)
{
	WIDTH = width;
	HEIGHT = height;

	glViewport(0, 0, WIDTH, HEIGHT);
	arcBall = ArcBall((GLfloat)WIDTH, (GLfloat)HEIGHT);
}