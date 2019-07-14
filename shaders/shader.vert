#version 450

layout(location=0) in vec3 vertex;
layout(location=1) in vec3 normal;

layout(std140, set=0, binding=0) uniform uniformMvp
{
	mat4 mvp;
};

layout(std140, set=0, binding=1) uniform uniformNm
{
	mat4 nm;
};

layout(location=0) out vec3 varying_normal;
layout(location=1) out vec2 varying_coord;

void main()
{
	varying_coord  = vec2(vertex.x/10.0 + 0.5,
	                      vertex.y/10.0 + 0.5);
	varying_normal = normalize(mat3(nm)*normal);
	gl_Position    = mvp*vec4(vertex, 1.0);
}
