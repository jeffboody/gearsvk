#version 450

layout(location=0) in vec3 varying_normal;

layout(std140, set=0, binding=2) uniform uniformColor
{
	vec4 color;
};

layout(location=0) out vec4 fragColor;

void main()
{
	vec4 ambient        = vec4(0.2, 0.2, 0.2, 1.0);
	vec3 light_position = vec3(5.0, 5.0, 10.0);
	light_position      = normalize(light_position);

	float ndotlp  = dot(varying_normal, light_position);
	if(ndotlp > 0.0)
	{
		vec4 diffuse = vec4(ndotlp, ndotlp, ndotlp, 0.0);
		fragColor    = color*(ambient + diffuse);
	}
	else
	{
		fragColor = color*ambient;
	}
}
