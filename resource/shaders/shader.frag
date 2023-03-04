#version 450

layout(location=0) in vec3 varying_normal;
layout(location=1) in vec2 varying_coord;

layout(std140, set=0, binding=2) uniform uniformColor
{
	vec4 color;
};

layout(set=0, binding=3) uniform sampler2D lava_sampler;

layout(location=0) out vec4 fragColor;

void main()
{
	vec3 ambient        = vec3(0.2, 0.2, 0.2);
	vec3 color3         = vec3(color);
	vec3 light_position = vec3(5.0, 5.0, 10.0);
	light_position      = normalize(light_position);

	vec3 lava = vec3(texture(lava_sampler, varying_coord));

	float ndotlp  = dot(varying_normal, light_position);
	if(ndotlp > 0.0)
	{
		vec3 diffuse = vec3(ndotlp, ndotlp, ndotlp);
		fragColor    = vec4(lava*color3*(ambient + diffuse), 1.0);
	}
	else
	{
		fragColor = vec4(lava*color3*ambient, 1.0);
	}
}
