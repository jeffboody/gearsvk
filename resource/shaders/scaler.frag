#version 450

layout(location=0) in vec4 varying_xyuv;

layout(set=0, binding=1) uniform sampler2D image;

layout(location=0) out vec4 fragColor;

void main()
{
	fragColor = texture(image, varying_xyuv.zw);
}
