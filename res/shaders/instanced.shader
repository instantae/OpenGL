#shader vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 aNormal;

layout(std430, binding = 0) buffer modelMatrices
{
	mat4 model[];
};

layout(std140, binding = 1) uniform Matrices
{
	mat4 projection;
	mat4 view;
};

layout(std140, binding = 2) buffer Colors
{
	vec4 color[];
};

/*out VS_OUT
{
	vec3 color;
} vs_out;
*/

uniform mat4 u_ModelMatrix;

out vec4 Color;
out vec3 FragPos;
out vec3 Normal;

void main()
{
	gl_Position = projection * view * model[gl_InstanceID] * vec4(position, 1.0);
	FragPos = vec3(model[gl_InstanceID] * vec4(position, 1.0));
	Normal = mat3(transpose(inverse(model[gl_InstanceID]))) * aNormal;
	Color = color[gl_InstanceID];
};

#shader fragment
#version 460 core

layout(location = 0) out vec4 out_color;

/*in VS_OUT
{
	vec3 color;
} fs_in;*/

in vec4 Color;
in vec3 Normal;
in vec3 FragPos;

uniform vec4 u_LightColor;
uniform vec3 u_lightpos;
uniform float u_PointLight_Constant;
uniform float u_PointLight_Linear;
uniform float u_PointLight_Quadratic;

uniform vec3 u_viewpos;
uniform float u_specularstrength;
uniform float u_specularshininess;

void main()
{
	vec3 LightColorNoAlpha = vec3(u_LightColor.r, u_LightColor.g, u_LightColor.b);

	

	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * LightColorNoAlpha;

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(u_lightpos - FragPos);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * LightColorNoAlpha;

	vec3 viewDir = normalize(u_viewpos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_specularshininess);
	vec3 specular = u_specularstrength * spec * LightColorNoAlpha;

	float distance = length(u_lightpos - FragPos);
	float attenuation = 1.0 / (u_PointLight_Constant + u_PointLight_Linear * distance + u_PointLight_Quadratic * (distance * distance));

	ambient *= attenuation * 10;
	diffuse *= attenuation;
	specular *= attenuation;


	vec4 result = vec4(ambient + diffuse + specular, u_LightColor.a) * Color;
	out_color = result;
};