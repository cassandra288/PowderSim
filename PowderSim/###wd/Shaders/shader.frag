#version 460 core

layout (std140, binding = 3) uniform objectData
{
	vec3 colorIn;
};

out vec3 color;

void main()
{
	color = colorIn;
}