#version 460 core

layout(location = 0) in vec2 position;

layout (std140, binding = 0) uniform coreRenderData
{
	vec2 viewportDims;
};

layout (std140, binding = 2) uniform coreObjectData
{
	mat3x3 modelMatrix;
};

layout (std140, binding = 3) uniform objectData
{
	vec2 _pos[1296];
	vec3 _color[1296];
};

out flat uint inID;

void main()
{
	vec3 vertPos = vec3(position, 1.f);
	vertPos.x += _pos[gl_InstanceID].x;
	vertPos.y += _pos[gl_InstanceID].y;
	
	inID = gl_InstanceID;
	gl_Position.x = (vertPos.x) / viewportDims.x;
	gl_Position.y = (vertPos.y) / viewportDims.y;
	gl_Position.zw = vec2(0.0, 1.0);
}
