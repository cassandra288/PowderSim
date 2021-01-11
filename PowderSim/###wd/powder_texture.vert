#version 460 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

out vec2 frag_uv;

layout (std140, binding = 0) uniform coreRenderData
{
	vec2 viewportDims;
	vec2 viewportOffset;
};

layout (std140, binding = 2) uniform coreObjectData
{
	mat3x3 modelMatrix;
};

void main()
{
	vec3 vertPos = modelMatrix * vec3(position, 1.f);
	vertPos - vec3(viewportOffset, 0.f);
	
	gl_Position.x = vertPos.x / viewportDims.x;
	gl_Position.y = vertPos.y / viewportDims.y;
	gl_Position.zw = vec2(0.0, 1.0);

	frag_uv = uv;
}
