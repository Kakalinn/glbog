#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

smooth out vec4 next_color;

uniform mat4 perspective;
uniform mat4 center_camera;
uniform mat4 rotate_camera;
uniform mat4 place_in_world;

void main()
{
	gl_Position = perspective*(rotate_camera*(center_camera*(place_in_world*vec4(position, 1.0))));
	//gl_Position = perspective*(center_camera*(place_in_world*vec4(position, 1.0)));
	//gl_Position = perspective*(place_in_world*vec4(position, 1.0));
	next_color = color;
}

