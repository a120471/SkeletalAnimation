#pragma once

#include <string>
#include <vector>
#include <map>

// GL Includes
#define GLEW_STATIC
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "objectShaderProgram.h"
#include "textureClass.h"
#include "utils.h"
#include "kinectDataClass.h"

using namespace std;

typedef struct _Joint Joint;

class Mesh
{
private:
	enum BUFFER_TYPES
	{
		INDEX_BUFFER,
		VERT_VB,
		NORMAL_VB,
		TEXCOORD_VB,
		BONE_VB,
		ELEMENTNUMS
	};

#define INVALID_MATERIAL 0xFFFFFFFF
	struct MeshEntry
	{
		MeshEntry()
		{
			numIdxs = 0;
			baseVert = 0;
			baseIdx = 0;
			materialIdx = INVALID_MATERIAL;
		}

		unsigned int numIdxs;
		unsigned int baseVert;
		unsigned int baseIdx;
		unsigned int materialIdx;
	};

#define NUM_BONES_PER_VEREX 6
	struct VertexBoneData
	{
		GLuint IDs[NUM_BONES_PER_VEREX];
		float weights[NUM_BONES_PER_VEREX];

		VertexBoneData()
		{
			Reset();
		};

		void Reset()
		{
			ZERO_MEM(IDs);
			ZERO_MEM(weights);
		}

		void AddBoneData(GLuint boneID, float weight);
	};

	struct BoneInfo
	{
		glm::mat4 boneOffset;
		glm::mat4 finalTransformation;
		std::string boneType;

		BoneInfo()
		{
			boneOffset = glm::mat4(1.0f);
			finalTransformation = glm::mat4(1.0f);
			boneType = string("Unkown");
		}
	};

public:
	vector<glm::vec3>* vertices;
	vector<glm::vec3>* normals;
	vector<glm::vec2>* texCoords;
	vector<VertexBoneData>* bones;
	vector<GLuint>* indices;

	Mesh(const string& path);
	~Mesh();
	void UpdateSkeletonUsingKinect(Joint* jointData, bool kinectDataFlag);
	void Draw(ShaderProgram& shader);

	GLuint VAOKinectData, VBOKinectDataPos, VBOKinectDataColor;
	void setAndDraw(ShaderProgram& shader1, Joint* jointData, bool kinecDataFlag)
	{
		glLineWidth(3);
		glBindVertexArray(VAOKinectData);
		std::vector<glm::vec3> dataSet;
		std::vector<glm::vec3> colorSet;
		//for (unsigned int i = 0; i < boneInfo.size(); i++)
		//{
		//	//glm::mat4 tt = glm::rotate(glm::mat4(), (float)glm::radians(-90.0f), glm::vec3(1, 0, 0)) * glm::inverse(boneInfo[i].boneOffset);
		//	glm::mat4 tt = boneInfo[i].finalTransformation * glm::inverse(boneInfo[i].boneOffset);

		//	//dataSet.push_back(glm::vec3(tt[3]));
		//	//colorSet.push_back(glm::vec3(0, 1, 0));
		//	//dataSet.push_back(glm::vec3(tt[3] + tt[0]));
		//	//colorSet.push_back(glm::vec3(0, 1, 0));

		//	dataSet.push_back(glm::vec3(tt[3]));
		//	colorSet.push_back(glm::vec3(1, 0, 0));
		//	dataSet.push_back(glm::vec3(tt[3] + tt[1]));
		//	colorSet.push_back(glm::vec3(1, 0, 0));

		//	//dataSet.push_back(glm::vec3(tt[3]));
		//	//colorSet.push_back(glm::vec3(0, 0, 1));
		//	//dataSet.push_back(glm::vec3(tt[3] + tt[2]));
		//	//colorSet.push_back(glm::vec3(0, 0, 1));
		//}

		//// Load data into vertex buffers
		//glBindBuffer(GL_ARRAY_BUFFER, VBOKinectDataPos);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(dataSet[0]) * dataSet.size(), &dataSet[0], GL_STATIC_DRAW);
		//glEnableVertexAttribArray(0);
		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//glBindBuffer(GL_ARRAY_BUFFER, VBOKinectDataColor);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(colorSet[0]) * colorSet.size(), &colorSet[0], GL_STATIC_DRAW);
		//glEnableVertexAttribArray(1);
		//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		//glDrawArrays(GL_LINES, 0, dataSet.size());


		if (kinecDataFlag)
		{
			dataSet.clear();
			colorSet.clear();
			glm::vec3 color123(1, 1, 0);
			float scale1123 = 1.0f;
			for (unsigned int i = 0; i < boneInfo.size(); i++)
			{
				glm::ivec2 jointST = skeletonJoint[boneInfo[i].boneType];

				dataSet.push_back(glm::vec3(-jointData[jointST.x].Position.X, jointData[jointST.x].Position.Y, -jointData[jointST.x].Position.Z) * scale1123);
				colorSet.push_back(color123);

				dataSet.push_back(glm::vec3(-jointData[jointST.y].Position.X, jointData[jointST.y].Position.Y, -jointData[jointST.y].Position.Z) * scale1123);
				colorSet.push_back(color123);
			}

			// Load data into vertex buffers
			glBindBuffer(GL_ARRAY_BUFFER, VBOKinectDataPos);
			glBufferData(GL_ARRAY_BUFFER, sizeof(dataSet[0]) * dataSet.size(), &dataSet[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glBindBuffer(GL_ARRAY_BUFFER, VBOKinectDataColor);
			glBufferData(GL_ARRAY_BUFFER, sizeof(colorSet[0]) * colorSet.size(), &colorSet[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glDrawArrays(GL_LINES, 0, dataSet.size());
		}

		glBindVertexArray(0);
	}


private:
	GLuint VAO;
	GLuint otherBuffers[ELEMENTNUMS];
	
	vector<MeshEntry> entries;
	vector<Texture*> textures;
	
	map<string, GLuint> boneMapping; // maps a bone name to its index
	GLuint numBones;
	vector<BoneInfo> boneInfo;
	map<string, glm::ivec2> skeletonJoint;

	const aiScene* pAiScene;
	Assimp::Importer importer;

	bool LoadModel(const string& path);
	bool InitFromScene(const aiScene* pAiScene, const string& path);
	void InitMesh(GLuint meshIndex, const aiMesh* pAiMesh);
	void LoadBones(GLuint meshIndex, const aiMesh* pAiMesh);
	bool InitMaterials(const aiScene* pAiScene, const string& filename);
	void SetupMesh();

	inline glm::mat4 GLMMat4FromAiMat4(const aiMatrix4x4 &input);

	void ReadNodeHeirarchy(const aiNode* pAiNode, const glm::mat4& parentTransform, Joint* jointData, bool kinectDataFlag);
};
