#version 460 core

layout(location = 0) in vec2 position;

void main()
{
	gl_Position.xy = position;
	gl_Position.zw = vec2(0.0, 1.0);
}
