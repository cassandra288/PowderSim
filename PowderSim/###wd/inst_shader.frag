#version 460 core

layout (std140, binding = 3) uniform objectData
{
	vec2 _pos[1296];
	vec3 _color[1296];
};

in flat uint inID;

out vec3 color;

void main()
{
	color = _color[inID];
}