#version 450

in vec3 next_pos;
in vec4 next_color;
in vec3 next_normal;

out vec4 out_color;

uniform float light_attn;
uniform vec3 light_pos;
uniform vec4 light_intensity;
uniform vec4 light_ambient;

void main()
{
	vec3 light_dir = light_pos - next_pos;
	float dist = dot(light_dir, light_dir);
	vec4 attn_light_intensity = light_intensity*(1/(1.0 + light_attn*dist));

    float ang = dot(normalize(next_normal), normalize(light_dir));
    ang = clamp(ang, 0, 1);
    
    out_color = next_color*attn_light_intensity*ang + next_color*light_ambient;
	//out_color = next_color;
	//out_color = vec4(ang, ang, ang, 1.0);
	//out_color = vec4(next_normal, 1.0);
	//out_color = vec4(light_dir, 1.0);
	//out_color = vec4(next_pos, 1.0);
	//out_color = vec4(dist/1000, dist/1000, dist/1000, 1.0);
}
