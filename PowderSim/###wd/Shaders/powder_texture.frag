#version 460 core

layout (binding=0) uniform sampler2D powderTexture;
in vec2 frag_uv;

out vec4 color;

void main()
{
	color = texture(powderTexture, frag_uv);
}