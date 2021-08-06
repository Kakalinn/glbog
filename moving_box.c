#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "thing.h"

GLFWwindow* window;

void mysleep(int ms)
{
	long startd = 1000*clock()/1000;

	while (!glfwWindowShouldClose(window) && 1000*clock()/1000 - startd < ms)
		glfwPollEvents();
}

GLuint load_shader(char *location, GLenum type)
{ // Reads, compiles and return the shader located at |location|.
	FILE* file = fopen(location, "r");
	assert(file);
	int i = 0, n = 1024;
	char *shader = malloc(n);
	do
	{
		if (i == n) shader = realloc(shader, n *= 2);
		shader[i] = fgetc(file);
	} while (shader[i++] != EOF);
	shader[i - 1] = '\0';
	fclose(file);
	GLuint ret = glCreateShader(type);
	glShaderSource(ret, 1, (const char**)(&shader), NULL);
	glCompileShader(ret);
	GLint status;
	glGetShaderiv(ret, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		char buffer[512];
		glGetShaderInfoLog(ret, 512, NULL, buffer);
		printf("Error compiling the shader '%s'.\n%s\n", location, buffer);
		exit(4);
	}
	free(shader);
	return ret;
}



GLuint load_program(GLuint vertex_shader, GLuint fragment_shader)
{
	GLuint ret = glCreateProgram();
	glAttachShader(ret, vertex_shader);
	glAttachShader(ret, fragment_shader);
	glLinkProgram(ret);

	GLint status;
    glGetProgramiv(ret, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
		char buffer[512];
		glGetProgramInfoLog(ret, 512, NULL, buffer);
		printf("Error compiling the program.\n%s\n", buffer);
		exit(5);
    }

	glDetachShader(ret, vertex_shader);
	glDetachShader(ret, fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	return ret;
}

void init(int width, int height) 
{
	assert(glfwInit() != -1);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


	window = glfwCreateWindow(width, height, "Test", NULL, NULL);
	
	assert(window);

	glfwMakeContextCurrent(window);
	assert(glewInit() == GLEW_OK);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
}

void end()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

int main()
{
	init(640, 480);
	GLuint vert, frag, prog;
	thing* box = create_thing("data/box.thing");

	frag = load_shader("shaders/default.frag", GL_FRAGMENT_SHADER);
	vert = load_shader("shaders/default.vert", GL_VERTEX_SHADER);
	prog = load_program(vert, frag);
	thing_set_program(box, prog);

	thing_move(box, 0.0, 0.0, -3.0);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		// handle inputs
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) thing_move(box, 0.0, 0.1, 0.0);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) thing_move(box, -0.1, 0.0, 0.0);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) thing_move(box, 0.0, -0.1, 0.0);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) thing_move(box, 0.1, 0.0, 0.0);
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) thing_move(box, 0.0, 0.0, 0.1);
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) thing_move(box, 0.0, 0.0, -0.1);

		// render
#if 1
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		thing_render(box);
		glfwSwapBuffers(window);
#endif
	}

	delete_thing(box);
	glDeleteProgram(prog);
	glDeleteShader(frag);
	glDeleteShader(vert);
	end();
	return 0;
}

