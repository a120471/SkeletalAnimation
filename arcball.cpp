/** KempoApi: The Turloc Toolkit *****************************/
/** *    *                                                  **/
/** **  **  Filename: arcBall.cpp                           **/
/**   **    Version:  Common                                **/
/**   **                                                    **/
/**                                                         **/
/**  Arcball class for mouse manipulation.                  **/
/**                                                         **/
/**                                                         **/
/**                                                         **/
/**                                                         **/
/**                              (C) 1999-2003 Tatewake.com **/
/**   History:                                              **/
/**   08/17/2003 - (TJG) - Creation                         **/
/**   09/23/2003 - (TJG) - Bug fix and optimization         **/
/**   09/25/2003 - (TJG) - Version for NeHe Basecode users  **/
/**                                                         **/
/*************************************************************/

// This File Has Been Modified !!! The Source Version Is Really Well Annotated

#include "arcBall.h"

ArcBall::ArcBall(GLfloat width, GLfloat height)
{
	this->type = None;

	// Set initial bounds
	this->SetBounds(width, height);
}

void ArcBall::Click(const glm::vec2 ptScreen, MotionType type)
{
	this->stPt = ptScreen;
	this->edPt = ptScreen;
	this->stVec = this->MapToSphere(ptScreen);
	this->edVec = this->stVec;
	this->type = type;
}

void ArcBall::Drag(const glm::vec2 ptScreen)
{
	this->edPt = ptScreen;
	this->edVec = this->MapToSphere(ptScreen);

	// Return the quaternion equivalent to the rotation
	glm::vec3 perp = glm::cross(stVec, edVec);

	// Compute the length of the perpendicular vector
	if (perp.length() > Epsilon)    //if its non-zero
	{
		// return the perpendicular vector as the transform after all
		this->quat.x = perp.x;
		this->quat.y = perp.y;
		this->quat.z = perp.z;
		// In the quaternion values, w is cosine (theta / 2), where theta is rotation angle
		this->quat.w = glm::dot(stVec, edVec);
	}
	else
	{
		// The begin and end vectors coincide, so return an identity transform
		this->quat = glm::vec4(0);
	}
}

// Map the point to the sphere
glm::vec3 ArcBall::MapToSphere(const glm::vec2 ptScreen)
{
	glm::vec2 ptTemp = ptScreen;
	// Adjust point coords and scale down to range of [-1 ... 1]
	ptTemp.x = (ptTemp.x * this->adjustWidth) - 1.0f;
	ptTemp.y = 1.0f - (ptTemp.y * this->adjustHeight);

	// Compute the square of the length of the vector to the point from the center
	GLfloat lengthSq = ptTemp.x * ptTemp.x + ptTemp.y * ptTemp.y;
	if (lengthSq > 1.0f)
	{
		GLfloat norm = 1.0f / sqrtf(lengthSq);
		return glm::vec3(ptTemp.x * norm, ptTemp.y * norm, 0);
	}
	else
	{
		return glm::vec3(ptTemp.x, ptTemp.y, sqrtf(1.0f - lengthSq));
	}
}

glm::mat4 ArcBall::GetMatrix()
{
	if (type == Rotation)
		return Mat4FromQuat4(this->quat);

	if (type == Scale)
	{
		glm::mat4 m = glm::mat4();
		//m[0][0] = m[1][1] = m[2][2] = 1.0f - (edPt.y - stPt.y) * adjustHeight; // y axis is upside down
		m[0][0] = m[1][1] = m[2][2] = 1.0f - (edPt.y - stPt.y) / 10.0f; // y axis is upside down
		return m;
	}

	if (type == Pan)
	{
		glm::mat4 m = glm::mat4();
		m[3][0] = edVec.x - stVec.x;
		m[3][1] = edVec.y - stVec.y;
		return m;
	}

	return glm::mat4();
}

void ArcBall::End()
{
	this->quat = glm::vec4(0);
	type = None;
}