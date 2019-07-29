#version 450

layout(location=0) in vec2 varying_coord;

layout(location=0) out vec4 fragColor;

void main()
{
	fragColor = vec4(varying_coord, 0.0, 1.0);
}
