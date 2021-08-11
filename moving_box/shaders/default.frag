#version 450

smooth in vec4 next_color;

out vec4 out_color;

void main()
{
	out_color = next_color;
	//out_color = vec4(1.0, 0.0, 0.0, 1.0);
}
