#version 450

layout(location=0) in vec3 vertex;
layout(set=0, binding=0) uniform uniformBuffer
{
	mat4 mvp;
};

void main()
{
    gl_Position = mvp*vec4(vertex, 1.0);
}
