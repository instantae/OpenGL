#shader vertex
#version 460 core

layout(location = 0) in vec4 position;

uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;


void main()
{
	gl_Position = u_ProjectionMatrix * u_ViewMatrix * u_ModelMatrix * position;
};

#shader fragment
#version 460 core

layout(location = 0) out vec4 out_color;


uniform vec3 u_LightColor;

void main()
{
	out_color = vec4(u_LightColor, 1.0);
};