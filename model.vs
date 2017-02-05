#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in ivec4 boneIDs1;
layout(location = 4) in vec4 weights1;
layout(location = 5) in ivec2 boneIDs2;
layout(location = 6) in vec2 weights2;

out vec2 ourTexCoord;
out vec3 ourNormal;
out vec3 ourWorldPos;

const int MAX_BONES = 100;
uniform mat4 gBones[MAX_BONES];

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform mat4 NormalMatrix;

void main()
{
	mat4 boneTransform = gBones[boneIDs1[0]] * weights1[0];
	boneTransform += gBones[boneIDs1[1]] * weights1[1];
	boneTransform += gBones[boneIDs1[2]] * weights1[2];
	boneTransform += gBones[boneIDs1[3]] * weights1[3];
	boneTransform += gBones[boneIDs2[0]] * weights2[0];
	boneTransform += gBones[boneIDs2[1]] * weights2[1];

	gl_Position = Projection * View * Model * boneTransform * vec4(position, 1.0f);
	ourTexCoord = texCoord;
	ourNormal = mat3(NormalMatrix * boneTransform) * normal;
	ourWorldPos = vec3(Model * vec4(position, 1.0f));
}