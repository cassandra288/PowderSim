#version 460 core

layout (std140, binding = 3) uniform objectData
{
	vec3 colorIn;
	vec3 test1;
	vec3 test2[2];
	mat2x2 test3;
	mat2x2 test4[2];
	struct
	{
		float p1;
		float p2;
	} a[2];
	float p3;
};

out vec3 color;

void main()
{
	color = vec3(p3, 0, 0);
}