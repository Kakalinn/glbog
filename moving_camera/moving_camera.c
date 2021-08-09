#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#ifndef WIDTH
#define WIDTH 640
#endif

#ifndef HEIGHT
#define HEIGHT 480
#endif


#include "mat4.h"
struct camera
{
	// |phi| is vertical angle, |theta| is horizontal.
	float x, y, z, phi, theta;
	 mat4 *r, *t;
} camera;
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

void set_camera_mat4()
{
	mat4_translate_scale(camera.t, -camera.x, -camera.y, -camera.z, 1.0, 1.0, 1.0);
	mat4_camera_rotation(camera.r, -camera.theta, -camera.phi);
}

void init()
{
	assert(glfwInit() != -1);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


	window = glfwCreateWindow(WIDTH, HEIGHT, "Test", NULL, NULL);
	
	assert(window);

	glfwMakeContextCurrent(window);
	assert(glewInit() == GLEW_OK);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0, 1.0);
	glEnable(GL_DEPTH_CLAMP);

	camera.x = camera.y = camera.z = camera.phi = camera.theta = 0.0;
	camera.r = mat4_create(NULL);
	camera.t = mat4_create(NULL);
}

void end()
{
	mat4_delete(camera.r);
	mat4_delete(camera.t);
	glfwDestroyWindow(window);
	glfwTerminate();
}

float clampf(float x, float l, float r)
{
	if (x < l) return l;
	if (x > r) return r;
	return x;
}

void move_camera(float x, float y, float z, float theta, float phi)
{
	camera.x += x, camera.y += y, camera.z += z, camera.theta += theta, camera.phi += phi;
	camera.theta = clampf(camera.theta, -M_PI, M_PI);
	camera.phi = fmodf(camera.phi, 2*M_PI);
}

int main()
{
	init();
	GLuint vert, frag, prog;
	thing* box1 = create_thing("data/box.thing");
	thing* box2 = create_thing("data/box.thing");

	frag = load_shader("shaders/default.frag", GL_FRAGMENT_SHADER);
	vert = load_shader("shaders/default.vert", GL_VERTEX_SHADER);
	prog = load_program(vert, frag);
	thing_set_program(box1, prog);
	thing_set_program(box2, prog);

	thing_move(box1, 0.0, 0.0, -3.0);
	thing_move(box2, 0.0, 0.0, -5.0);

	double time_start = glfwGetTime();
	int number_of_frames = 0;
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		// handle inputs
		float xx = 0.0, yy = 0.0, zz = 0.0, tt = 0.0, pp = 0.0, ss = sin(camera.phi), cc = cos(camera.phi);
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;
		if (glfwGetKey(window, GLFW_KEY_W)      == GLFW_PRESS) xx += -0.1*ss, zz += -0.1*cc;
		if (glfwGetKey(window, GLFW_KEY_A)      == GLFW_PRESS) xx += -0.1*cc, zz +=  0.1*ss;
		if (glfwGetKey(window, GLFW_KEY_S)      == GLFW_PRESS) xx +=  0.1*ss, zz +=  0.1*cc;
		if (glfwGetKey(window, GLFW_KEY_D)      == GLFW_PRESS) xx +=  0.1*cc, zz += -0.1*ss;
		if (glfwGetKey(window, GLFW_KEY_R)      == GLFW_PRESS) yy += 0.1;
		if (glfwGetKey(window, GLFW_KEY_F)      == GLFW_PRESS) yy += -0.1;
		if (glfwGetKey(window, GLFW_KEY_RIGHT)  == GLFW_PRESS) pp += -0.03;
		if (glfwGetKey(window, GLFW_KEY_LEFT)   == GLFW_PRESS) pp += 0.03;
		if (glfwGetKey(window, GLFW_KEY_DOWN)   == GLFW_PRESS) tt += -0.03;
		if (glfwGetKey(window, GLFW_KEY_UP)     == GLFW_PRESS) tt += 0.03;
		move_camera(xx, yy, zz, tt, pp);

		// do camera
		set_camera_mat4();

		// render
#if 1
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		thing_render(box1);
		thing_render(box2);
		glfwSwapBuffers(window);
#endif
		number_of_frames++;
	}
	printf("A total of %d frames were rendered in %f time\n", number_of_frames, glfwGetTime() - time_start);

	delete_thing(box1);
	glDeleteProgram(prog);
	glDeleteShader(frag);
	glDeleteShader(vert);
	end();
	return 0;
}

