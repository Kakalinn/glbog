#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

smooth out vec4 next_color;

uniform vec3 offset;
uniform mat4 perspective;

void main()
{
	gl_Position = perspective*(vec4(position + offset, 1.0));
	//gl_Position = vec4(position + offset, 1.0);
	//gl_Position = vec4(position, 1.0);
	next_color = color;
	//next_color = vec4(1.0, 0.0, 0.0, 1.0);
}

