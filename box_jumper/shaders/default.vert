#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 normal;

out vec3 next_pos;
out vec4 next_color;
out vec3 next_normal;

uniform mat4 perspective;
uniform mat4 center_camera;
uniform mat4 rotate_camera;
uniform mat4 place_in_world;

void main()
{
	gl_Position = perspective*(rotate_camera*(center_camera*(place_in_world*vec4(pos, 1.0))));

	next_color = color;
	next_pos = (place_in_world*vec4(pos, 1.0)).xyz;
	next_normal = normal;
}

