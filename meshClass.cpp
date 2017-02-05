#include "meshClass.h"

#include <string>
#include <sstream>
#include <iostream>

#include <stb_image.h>

#define POSITION_LOCATION		0
#define NORMAL_LOCATION			1
#define TEX_COORD_LOCATION		2
#define BONE_ID_LOCATION1		3
#define BONE_WEIGHT_LOCATION1	4
#define BONE_ID_LOCATION2		5
#define BONE_WEIGHT_LOCATION2	6

void Mesh::VertexBoneData::AddBoneData(GLuint boneID, float weight)
{
	for (GLuint i = 0; i < ARRAY_SIZE_IN_ELEMENTS(IDs); i++)
	{
		if (weights[i] == 0.0)
		{
			IDs[i] = boneID;
			weights[i] = weight;
			return;
		}
	}

	// should never get here - more bones than we have space for
	assert(0); // this will let program exit;
}

Mesh::Mesh(const string& path)
{
	glGenVertexArrays(1, &VAOKinectData);
	glGenBuffers(1, &VBOKinectDataPos);
	glGenBuffers(1, &VBOKinectDataColor);

	pAiScene = NULL;

	vertices = new vector<glm::vec3>;
	normals = new vector<glm::vec3>;
	texCoords = new vector<glm::vec2>;
	bones = new vector<VertexBoneData>;
	indices = new vector<GLuint>;

	this->LoadModel(path);
}

Mesh::~Mesh()
{
	SafeDelete(vertices);
	SafeDelete(normals);
	SafeDelete(texCoords);
	SafeDelete(bones);
	SafeDelete(indices);

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(otherBuffers), otherBuffers);

	glDeleteVertexArrays(1, &VAOKinectData);
	glDeleteBuffers(1, &VBOKinectDataPos);
	glDeleteBuffers(1, &VBOKinectDataColor);
}

bool Mesh::LoadModel(const string& path)
{
	// Read file via ASSIMP
	pAiScene = importer.ReadFile(path, aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_GenNormals);

	bool returnValue = false;
	// Check for errors
	if (!pAiScene || pAiScene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !pAiScene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		returnValue = false;
		return returnValue;
	}
	else
		returnValue = InitFromScene(pAiScene, path);

	//if (returnValue)
		SetupMesh();
	return returnValue;
}

bool Mesh::InitFromScene(const aiScene* pAiScene, const string& path)
{
	entries.resize(pAiScene->mNumMeshes);
	textures.resize(pAiScene->mNumMaterials);

	GLuint numVertices = 0;
	GLuint numIndices = 0;

	// Count the number of vertices and indices
	for (GLuint i = 0; i < entries.size(); i++)
	{
		entries[i].materialIdx = pAiScene->mMeshes[i]->mMaterialIndex;
		entries[i].numIdxs = pAiScene->mMeshes[i]->mNumFaces * 3;
		entries[i].baseVert = numVertices;
		entries[i].baseIdx = numIndices;

		numVertices += pAiScene->mMeshes[i]->mNumVertices;
		numIndices += entries[i].numIdxs;
	}

	// Reserve space in the vectors for the vertex attributes and indices
	vertices->reserve(numVertices);
	normals->reserve(numVertices);
	texCoords->reserve(numVertices);
	bones->resize(numVertices);
	indices->reserve(numIndices);

	// Initialize the meshes in the scene one by one
	for (GLuint i = 0; i < entries.size(); i++)
	{
		const aiMesh* pAiMesh = pAiScene->mMeshes[i];
		InitMesh(i, pAiMesh);
	}

	if (!InitMaterials(pAiScene, path))
		return false;

	return true;
}

void Mesh::InitMesh(GLuint meshIndex, const aiMesh* pAiMesh)
{
	const aiVector3D zero3D(0.0f, 0.0f, 0.0f);

	// Populate the vertex attribute vectors
	for (GLuint i = 0; i < pAiMesh->mNumVertices; i++)
	{
		const aiVector3D* pPos = &(pAiMesh->mVertices[i]);
		const aiVector3D* pNormal = &(pAiMesh->mNormals[i]); // error if this pointer is NULL
		const aiVector3D* pTexCoord = pAiMesh->HasTextureCoords(0) ? &(pAiMesh->mTextureCoords[0][i]) : &zero3D;

		vertices->push_back(glm::vec3(pPos->x, pPos->y, pPos->z));
		normals->push_back(glm::vec3(pNormal->x, pNormal->y, pNormal->z));
		texCoords->push_back(glm::vec2(pTexCoord->x, pTexCoord->y));
	}

	LoadBones(meshIndex, pAiMesh);

	// Populate the index buffer
	for (GLuint i = 0; i < pAiMesh->mNumFaces; i++)
	{
		const aiFace& face = pAiMesh->mFaces[i];
		assert(face.mNumIndices == 3);
		indices->push_back(face.mIndices[0]);
		indices->push_back(face.mIndices[1]);
		indices->push_back(face.mIndices[2]);
	}
}

glm::mat4 Mesh::GLMMat4FromAiMat4(const aiMatrix4x4 &a)
{
	return glm::mat4(a.a1, a.b1, a.c1, a.d1, // 1st column
		a.a2, a.b2, a.c2, a.d2,
		a.a3, a.b3, a.c3, a.d3,
		a.a4, a.b4, a.c4, a.d4);
}

void Mesh::LoadBones(GLuint meshIndex, const aiMesh* pAiMesh)
{
	skeletonJoint.clear();
	// Torso
	skeletonJoint[string("spine1")] = glm::ivec2(JointType::JointType_SpineMid, JointType::JointType_SpineShoulder);
	skeletonJoint[string("spine2")] = glm::ivec2(JointType::JointType_SpineMid, JointType::JointType_SpineBase);
	skeletonJoint[string("neck")] = glm::ivec2(JointType::JointType_SpineShoulder, JointType::JointType_Neck);
	skeletonJoint[string("head")] = glm::ivec2(JointType::JointType_Neck, JointType::JointType_Head);
	// Left Arm   
	skeletonJoint[string("shoulderL")] = glm::ivec2(JointType::JointType_SpineShoulder, JointType::JointType_ShoulderLeft);
	skeletonJoint[string("armUpL")] = glm::ivec2(JointType::JointType_ShoulderLeft, JointType::JointType_ElbowLeft);
	skeletonJoint[string("armDownL")] = glm::ivec2(JointType::JointType_ElbowLeft, JointType::JointType_WristLeft);
	skeletonJoint[string("palmL")] = glm::ivec2(JointType::JointType_WristLeft, JointType::JointType_HandLeft);
	skeletonJoint[string("fingerL")] = glm::ivec2(JointType::JointType_HandLeft, JointType::JointType_HandTipLeft);
	skeletonJoint[string("thumbL")] = glm::ivec2(JointType::JointType_WristLeft, JointType::JointType_ThumbLeft);
	// Right Arm
	skeletonJoint[string("shoulderR")] = glm::ivec2(JointType::JointType_SpineShoulder, JointType::JointType_ShoulderRight);
	skeletonJoint[string("armUpR")] = glm::ivec2(JointType::JointType_ShoulderRight, JointType::JointType_ElbowRight);
	skeletonJoint[string("armDownR")] = glm::ivec2(JointType::JointType_ElbowRight, JointType::JointType_WristRight);
	skeletonJoint[string("palmR")] = glm::ivec2(JointType::JointType_WristRight, JointType::JointType_HandRight);
	skeletonJoint[string("fingerR")] = glm::ivec2(JointType::JointType_HandRight, JointType::JointType_HandTipRight);
	skeletonJoint[string("thumbR")] = glm::ivec2(JointType::JointType_WristRight, JointType::JointType_ThumbRight);
	// Left Leg
	skeletonJoint[string("hipBoneL")] = glm::ivec2(JointType::JointType_SpineBase, JointType::JointType_HipLeft);
	skeletonJoint[string("legUpL")] = glm::ivec2(JointType::JointType_HipLeft, JointType::JointType_KneeLeft);
	skeletonJoint[string("legDownL")] = glm::ivec2(JointType::JointType_KneeLeft, JointType::JointType_AnkleLeft);
	skeletonJoint[string("footL")] = glm::ivec2(JointType::JointType_AnkleLeft, JointType::JointType_FootLeft);
	// Right Leg
	skeletonJoint[string("hipBoneR")] = glm::ivec2(JointType::JointType_SpineBase, JointType::JointType_HipRight);
	skeletonJoint[string("legUpR")] = glm::ivec2(JointType::JointType_HipRight, JointType::JointType_KneeRight);
	skeletonJoint[string("legDownR")] = glm::ivec2(JointType::JointType_KneeRight, JointType::JointType_AnkleRight);
	skeletonJoint[string("footR")] = glm::ivec2(JointType::JointType_AnkleRight, JointType::JointType_FootRight);

	for (GLuint i = 0; i < pAiMesh->mNumBones; i++)
	{
		GLuint boneIdx = 0;

		string boneName(pAiMesh->mBones[i]->mName.data);
		if (boneMapping.find(boneName) == boneMapping.end())
		{
			// Allocate an index for a new bone
			boneIdx = numBones++;
			boneMapping[boneName] = boneIdx;

			BoneInfo bi;
			boneInfo.push_back(bi);
			boneInfo[boneIdx].boneOffset = GLMMat4FromAiMat4(pAiMesh->mBones[i]->mOffsetMatrix);
			boneInfo[boneIdx].boneType = boneName;
		}
		else
			boneIdx = boneMapping[boneName];

		for (GLuint j = 0; j < pAiMesh->mBones[i]->mNumWeights; j++)
		{
			GLuint vertID = entries[meshIndex].baseVert + pAiMesh->mBones[i]->mWeights[j].mVertexId;
			float weight = pAiMesh->mBones[i]->mWeights[j].mWeight;
			bones->at(vertID).AddBoneData(boneIdx, weight);
		}
	}
}

bool Mesh::InitMaterials(const aiScene* pAiScene, const string& filename)
{
	// Extract the directory part from the file name
	string::size_type dlashIdx = filename.find_last_of("/");
	string dir;

	if (dlashIdx == string::npos)
		dir = ".";
	else if (dlashIdx == 0)
		dir = "/";
	else
		dir = filename.substr(0, dlashIdx);

	bool returnValue = true;

	// Initialize the materials
	for (GLuint i = 0; i < pAiScene->mNumMaterials; i++)
	{
		const aiMaterial* pAiMaterial = pAiScene->mMaterials[i];

		textures[i] = NULL;

		if (pAiMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString path;

			if (pAiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				std::string p(path.data);
				if (p.substr(0, 2) == ".\\")
				{
					p = p.substr(2, p.size() - 2);
				}

				std::string fullPath = dir + "/" + p;

				textures[i] = new Texture(GL_TEXTURE_2D, fullPath.c_str());

				if (!textures[i]->Load())
				{
					printf("Error loading texture '%s'\n", fullPath.c_str());
					SafeDelete(textures[i]);
					returnValue = false;
				}
				else
					printf("%d - loaded texture '%s'\n", i, fullPath.c_str());
			}
		}
	}

	return returnValue;
}

// Initializes all the buffer objects/arrays
void Mesh::SetupMesh()
{
	// Create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(otherBuffers), otherBuffers);
	
	glBindVertexArray(VAO);
	// Load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, otherBuffers[VERT_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices->at(0)) * vertices->size(), &vertices->at(0), GL_STATIC_DRAW);
	glEnableVertexAttribArray(POSITION_LOCATION);
	glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, otherBuffers[NORMAL_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals->at(0)) * normals->size(), &normals->at(0), GL_STATIC_DRAW);
	glEnableVertexAttribArray(NORMAL_LOCATION);
	glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, otherBuffers[TEXCOORD_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords->at(0)) * texCoords->size(), &texCoords->at(0), GL_STATIC_DRAW);
	glEnableVertexAttribArray(TEX_COORD_LOCATION);
	glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, otherBuffers[BONE_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bones->at(0)) * bones->size(), &bones->at(0), GL_STATIC_DRAW);
	glEnableVertexAttribArray(BONE_ID_LOCATION1);
	glVertexAttribIPointer(BONE_ID_LOCATION1, 4, GL_UNSIGNED_INT, sizeof(VertexBoneData), (const GLvoid*)0);
	glEnableVertexAttribArray(BONE_WEIGHT_LOCATION1);
	glVertexAttribPointer(BONE_WEIGHT_LOCATION1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)offsetof(VertexBoneData, weights));
	glEnableVertexAttribArray(BONE_ID_LOCATION2);
	glVertexAttribIPointer(BONE_ID_LOCATION2, 2, GL_UNSIGNED_INT, sizeof(VertexBoneData), (const GLvoid*)(4 * sizeof(int)));
	glEnableVertexAttribArray(BONE_WEIGHT_LOCATION2);
	glVertexAttribPointer(BONE_WEIGHT_LOCATION2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)(offsetof(VertexBoneData, weights) + 4 * sizeof(float)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, otherBuffers[INDEX_BUFFER]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices->at(0)) * indices->size(), &indices->at(0), GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void Mesh::UpdateSkeletonUsingKinect(Joint* jointData, bool kinectDataFlag)
{	
	glm::mat4 identity = glm::mat4();
	ReadNodeHeirarchy(pAiScene->mRootNode, identity, jointData, kinectDataFlag);
}

void Mesh::ReadNodeHeirarchy(const aiNode* pAiNode, const glm::mat4& parentTransform, Joint* jointData, bool kinectDataFlag)
{
	string nodeName(pAiNode->mName.data);
	glm::mat4 nodeTransformation(GLMMat4FromAiMat4(pAiNode->mTransformation));
	glm::mat4 globalTransformation = parentTransform * nodeTransformation;
	
	if (boneMapping.find(nodeName) != boneMapping.end())
	{
		GLuint boneIdx = this->boneMapping[nodeName];

		if (kinectDataFlag && skeletonJoint.find(nodeName) != skeletonJoint.end())
		{
			glm::ivec2 jointST = skeletonJoint[nodeName];

			glm::vec3 jointS(-jointData[jointST.x].Position.X, jointData[jointST.x].Position.Y, -jointData[jointST.x].Position.Z);
			glm::vec3 jointE(-jointData[jointST.y].Position.X, jointData[jointST.y].Position.Y, -jointData[jointST.y].Position.Z);
			glm::vec3 curBoneDirection = jointE - jointS;
			curBoneDirection = glm::normalize(curBoneDirection);
			
			glm::mat4 bonePos = globalTransformation;
			glm::vec3 initialBoneDirection(bonePos[1][0], bonePos[1][1], bonePos[1][2]);
			initialBoneDirection = glm::normalize(initialBoneDirection);

			glm::vec3 rotationAxis = glm::cross(initialBoneDirection, curBoneDirection);
			float rotationTheta = glm::acos(glm::dot(initialBoneDirection, curBoneDirection));
						
			if (!strcmp(nodeName.c_str(), "spine1"))
			{
				//globalTransformation *= glm::length(jointE - jointS);
				globalTransformation[3] = glm::vec4(-jointData[jointST.x].Position.X, jointData[jointST.x].Position.Y, -jointData[jointST.x].Position.Z, 1.0f);
			}

			globalTransformation = 
				glm::translate(glm::mat4(), glm::vec3(bonePos[3][0], bonePos[3][1], bonePos[3][2])) *
				glm::rotate(glm::mat4(), rotationTheta, rotationAxis) *
				glm::translate(glm::mat4(), -glm::vec3(bonePos[3][0], bonePos[3][1], bonePos[3][2])) *
				globalTransformation;

			boneInfo[boneIdx].finalTransformation = globalTransformation * boneInfo[boneIdx].boneOffset;
		}
		else
		{
			globalTransformation += glm::mat4(rand() / 1000000.0f);

			boneInfo[boneIdx].finalTransformation = globalTransformation * boneInfo[boneIdx].boneOffset;
		}		
	}

	for (unsigned int i = 0; i < pAiNode->mNumChildren; i++)
		ReadNodeHeirarchy(pAiNode->mChildren[i], globalTransformation, jointData, kinectDataFlag);
}

// Render the mesh
void Mesh::Draw(ShaderProgram& shader)
{
	// Bind Skeletion
	for (GLuint i = 0; i < numBones; i++)
	{
		char Name[128];
		memset(Name, 0, sizeof(Name));
		_snprintf_s(Name, sizeof(Name), "gBones[%d]", i);

		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, Name), 1, GL_FALSE, glm::value_ptr(boneInfo[i].finalTransformation));
	}

	// Draw mesh
	glBindVertexArray(VAO);
	for (GLuint i = 0; i < entries.size(); i++)
	{
		const GLuint materialIdx = entries[i].materialIdx;
		assert(materialIdx < textures.size());

		if (textures[materialIdx])
			textures[materialIdx]->Bind(GL_TEXTURE0);

		glDrawElementsBaseVertex(GL_TRIANGLES, entries[i].numIdxs, GL_UNSIGNED_INT,
			(void*)(sizeof(GLuint) * entries[i].baseIdx), entries[i].baseVert);
	}
	//glDrawElements(GL_TRIANGLES, indices->size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	//// Always good practice to set everything back to defaults once configured.
	//for (GLuint i = 0; i < this->textures.size(); i++)
	//{
	//	glActiveTexture(GL_TEXTURE0 + i);
	//	glBindTexture(GL_TEXTURE_2D, 0);
	//}
}
