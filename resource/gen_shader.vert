#version 450

layout(location=0) in vec2 coords;

layout(std140, set=0, binding=0) uniform uniformMvp
{
	mat4 mvp;
};

layout(location=0) out vec2 varying_coord;

void main()
{
	varying_coord  = coords;
	gl_Position    = mvp*vec4(coords, -1.0, 1.0);
}
