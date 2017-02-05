#version 330 core

struct Material 
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};
uniform Material Material1;

struct Light 
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform Light Light1;

in vec2 ourTexCoord;
in vec3 ourNormal;
in vec3 ourWorldPos;

out vec4 color;

uniform sampler2D TextureDiffuse1;

uniform vec3 ViewPos;

void main()
{
	// Ambient
	vec3 ambient = Light1.ambient * Material1.ambient;

	// Diffuse 
	vec3 norm = normalize(ourNormal);
	vec3 lightDir = normalize(Light1.position - ourWorldPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = Light1.diffuse * (diff * Material1.diffuse);

	// Specular
	vec3 viewDir = normalize(ViewPos - ourWorldPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), Material1.shininess);
	vec3 specular = Light1.specular * (spec * Material1.specular);

	color = vec4(ambient + diffuse + specular, 1.0f);
	color *= vec4(texture(TextureDiffuse1, ourTexCoord));
}