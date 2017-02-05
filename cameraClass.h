#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


// Defines several possible options for camera movement
enum CameraMovement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 3.0f;
const GLfloat SENSITIVTY = 0.25f;
const GLfloat ZOOMVALUE = 45.0f;


// A camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// Camera Attributes
	glm::vec3 position;
	glm::vec3 front, up, right;
	glm::vec3 worldUp;
	// Eular Angles
	GLfloat yaw, pitch;
	// Camera options
	GLfloat moveSpeed;
	GLfloat mouseSensitivity;
	GLfloat zoomValue;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH)
		: position(position)
		, worldUp(worldUp)
		, yaw(yaw)
		, pitch(pitch)
		, moveSpeed(SPEED)
		, mouseSensitivity(SENSITIVTY)
		, zoomValue(ZOOMVALUE)
	{
		this->UpdateCameraVectors();
	}

	// Returns the view matrix calculated using Eular Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(this->position, this->position + this->front, this->up);
	}

	// Processes input received from any keyboard-like input system
	void ProcessKeyboard(CameraMovement direction, GLfloat deltaTime)
	{
		GLfloat dis = this->moveSpeed * deltaTime;
		if (direction == FORWARD)
			this->position += this->front * dis;
		if (direction == BACKWARD)
			this->position -= this->front * dis;
		if (direction == LEFT)
			this->position -= this->right * dis;
		if (direction == RIGHT)
			this->position += this->right * dis;
		// Make sure the user stays at the ground level
		this->position.y = 0.0f; // <-- this one-liner keeps the user at the ground level (xz plane)
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(GLfloat xOffset, GLfloat yOffset, GLboolean constrainPitch = true)
	{
		xOffset *= this->mouseSensitivity;
		yOffset *= this->mouseSensitivity;

		this->yaw += xOffset;
		this->pitch += yOffset;
		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (this->pitch > 89.0f)
				this->pitch = 89.0f;
			if (this->pitch < -89.0f)
				this->pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Eular angles
		this->UpdateCameraVectors();
	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(GLfloat yoffset)
	{
		if (this->zoomValue >= 1.0f && this->zoomValue <= 45.0f)
			this->zoomValue -= yoffset;
		if (this->zoomValue <= 1.0f)
			this->zoomValue = 1.0f;
		if (this->zoomValue >= 45.0f)
			this->zoomValue = 45.0f;
	}

private:
	// Update the Camera's vectors
	void UpdateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 frontT;
		frontT.x = cos(glm::radians(this->pitch)) * cos(glm::radians(this->yaw));
		frontT.y = sin(glm::radians(this->pitch));
		frontT.z = cos(glm::radians(this->pitch)) * sin(glm::radians(this->yaw));
		this->front = glm::normalize(frontT);

		// Also re-calculate the Right and Up vector
		this->right = glm::normalize(glm::cross(this->front, this->worldUp));
		this->up = glm::normalize(glm::cross(this->right, this->front));
	}
};