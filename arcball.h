/** KempoApi: The Turloc Toolkit *****************************/
/** *    *                                                  **/
/** **  **  Filename: arcBall.h                             **/
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
#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <glm/glm.hpp>

// assuming IEEE-754(GLfloat), which i believe has max precision of 7 bits
# define Epsilon 1.0e-5

class ArcBall
{
public:
	enum MotionType { None, Rotation, Pan, Scale };

	ArcBall(GLfloat width, GLfloat height);
	~ArcBall() {};

	// Set new bounds
	inline void SetBounds(GLfloat width, GLfloat height)
	{
		this->halfWidth = width;
		this->halfHeight = height;

		// Set adjustment factor for width/height
		this->adjustWidth = 1.0f / ((width - 1.0f) * 0.5f);
		this->adjustHeight = 1.0f / ((height - 1.0f) * 0.5f);
	}

	void Click(const glm::vec2 ptScreen, MotionType type);
	void Drag(const glm::vec2 ptScreen);

    inline glm::mat4 Mat4FromQuat4(const glm::vec4 q)
    {
		GLfloat n, s;
        GLfloat xs, ys, zs;
        GLfloat wx, wy, wz;
        GLfloat xx, xy, xz;
        GLfloat yy, yz, zz;
		
		n = glm::dot(q, q);
		s = (n > 0.0f) ? (2.0f / n) : 0.0f;
		if (n < Epsilon)
			return glm::mat4();

		xs = q.x * s;  ys = q.y * s;  zs = q.z * s;
		wx = q.w * xs; wy = q.w * ys; wz = q.w * zs;
		xx = q.x * xs; xy = q.x * ys; xz = q.x * zs;
		yy = q.y * ys; yz = q.y * zs; zz = q.z * zs;

		// column-major ordering
		glm::mat4 m = glm::mat4();
		m[0][0] = 1.0f - (yy + zz);	m[1][0] = xy - wz;  m[2][0] = xz + wy;
		m[0][1] = xy + wz;	m[1][1] = 1.0f - (xx + zz); m[2][1] = yz - wx;
		m[0][2] = xz - wy;  m[1][2] = yz + wx;  m[2][2] = 1.0f - (xx + yy);
		return m;
    }

	glm::mat4 GetMatrix();

	void End();

private:
	MotionType	type;
	glm::vec2	stPt, edPt;
	glm::vec3   stVec, edVec;
	glm::vec4	quat;
	GLfloat		halfWidth, halfHeight;
	GLfloat     adjustWidth, adjustHeight; // Mouse bounds width / height

	inline glm::vec3 MapToSphere(const glm::vec2 newPt);
};
