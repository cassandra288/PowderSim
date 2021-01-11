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

void main()
{
	vec3 vertPos = modelMatrix * vec3(position, 1.f);
	
	gl_Position.x = vertPos.x / viewportDims.x;
	gl_Position.y = vertPos.y / viewportDims.y;
	gl_Position.zw = vec2(0.0, 1.0);
}
