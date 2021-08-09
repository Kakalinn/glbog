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


int player_is_on_top_of_box(thing* box)
{ // |box| must be a box.
	float xl = box->x, xr = box->x + box->sx,
		  yl = box->y, yr = box->y + box->sy,
		  zl = box->z, zr = box->z + box->sz,
		  x = camera.x, y = camera.y, z = camera.z;
	float tol = 0.0;
	float height = 0.1;
	if (x < xl - tol) return 0;
	if (x > xr + tol) return 0;
	if (z < zl - tol) return 0;
	if (z > zr + tol) return 0;

	if (y > yr + height) return 0;
	if (y < yl - tol) return 0;
	return 1;
}

int player_is_hitting_box(thing* box)
{ // |box| must be a box.
	float xl = box->x, xr = box->x + box->sx,
		  yl = box->y, yr = box->y + box->sy,
		  zl = box->z, zr = box->z + box->sz,
		  x = camera.x, y = camera.y, z = camera.z;
	float tol = 0.11;
	printf(">> (%f %f %f)\n", x, y, z);
	printf(">> [%f %f]\n", xl, xr);
	printf(">> [%f %f]\n", yl, yr);
	printf(">> [%f %f]\n", zl, zr);
	printf("1\n");
	if (x < xl - tol) return 0;
	printf("2\n");
	if (x > xr + tol) return 0;
	printf("3\n");
	if (y < yl - tol) return 0;
	printf("4\n");
	if (y > yr - tol) return 0;
	printf("5\n");
	if (z < zl - tol) return 0;
	printf("6\n");
	if (z > zr + tol) return 0;
	printf("7\n");
	return 1;
}

int main()
{
	init();
	GLuint vert, frag, prog;
	int i, boxes_n = 3, grounded = 0;
	float xvel = 0.0, yvel = 0.0, zvel = 0.0, grav = -10.0, ymaxvel = 0.1;
	thing* box[boxes_n];
	box[0] = create_thing("data/floor.thing");
	box[1] = create_thing("data/box.thing");
	box[2] = create_thing("data/box.thing");

	frag = load_shader("shaders/default.frag", GL_FRAGMENT_SHADER);
	vert = load_shader("shaders/default.vert", GL_VERTEX_SHADER);
	prog = load_program(vert, frag);
	thing_set_program(box[0], prog);
	thing_set_program(box[1], prog);
	thing_set_program(box[2], prog);

	thing_move(box[0], -10.0, -2.1, -10.0);
	thing_move(box[1], 0.0, -1.1, -3.0);
	thing_move(box[2], 0.0, -1.1, -5.0);

	thing_scale(box[2], 2.0, 2.0, 2.0);
	thing_scale(box[0], 200.0, 1.0, 200.0);

	double time_current, time_elapsed, time_start = glfwGetTime(), time_last = glfwGetTime();
	int number_of_frames = 0;
	while (!glfwWindowShouldClose(window))
	{
		//printf(">>>>>> %f %f %f (%f %f %f)\n", camera.x, camera.y, camera.z, xvel, yvel, zvel);
		glfwPollEvents();
		time_current = glfwGetTime();
		time_elapsed = time_current - time_last;
		// handle inputs
		float xx = 0.0, yy = 0.0, zz = 0.0, tt = 0.0, pp = 0.0, ss = sin(camera.phi), cc = cos(camera.phi);
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;
		if (glfwGetKey(window, GLFW_KEY_W)      == GLFW_PRESS) xx += -6.0*ss, zz += -6.0*cc;
		if (glfwGetKey(window, GLFW_KEY_A)      == GLFW_PRESS) xx += -6.0*cc, zz +=  6.0*ss;
		if (glfwGetKey(window, GLFW_KEY_S)      == GLFW_PRESS) xx +=  6.0*ss, zz +=  6.0*cc;
		if (glfwGetKey(window, GLFW_KEY_D)      == GLFW_PRESS) xx +=  6.0*cc, zz += -6.0*ss;
		//if (glfwGetKey(window, GLFW_KEY_R)      == GLFW_PRESS) yy += 0.1;
		//if (glfwGetKey(window, GLFW_KEY_F)      == GLFW_PRESS) yy += -0.1;
		if (glfwGetKey(window, GLFW_KEY_RIGHT)  == GLFW_PRESS) pp += -2.0;
		if (glfwGetKey(window, GLFW_KEY_LEFT)   == GLFW_PRESS) pp +=  2.0;
		if (glfwGetKey(window, GLFW_KEY_DOWN)   == GLFW_PRESS) tt += -2.0;
		if (glfwGetKey(window, GLFW_KEY_UP)     == GLFW_PRESS) tt +=  2.0;

		// physics
		if (grounded)
		{
			for (i = 0; i < boxes_n; i++) if (player_is_on_top_of_box(box[i])) break;
			if (i == boxes_n) grounded = 0;
			if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) yvel = 4.0, grounded = 0;
		}
		else
		{
			yvel += clampf(time_elapsed*grav, -ymaxvel, ymaxvel);

			if (yvel < 0.0)
			{
				for (i = 0; i < boxes_n; i++) if (player_is_on_top_of_box(box[i]))
				{
					yvel = 0.0;
					camera.y = box[i]->y + box[i]->sy + 0.1;
					grounded = 1;
					break;
				}
			}
			if (i < boxes_n) printf(">> landed on box[%d]\n", i);

			yy = yvel;
		}

		//xz-plane collision, simple
		for (i = 0; i < boxes_n; i++) if (player_is_hitting_box(box[i])) break;
		if (i < boxes_n) 
		{
			printf("collinding with box[%d]\n", i);
			xx = 0.0, zz = 0.0;
		}


		move_camera(xx*time_elapsed, yy*time_elapsed, zz*time_elapsed, tt*time_elapsed, pp*time_elapsed);


		// do camera
		set_camera_mat4();

		// render
#if 1
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (i = 0; i < boxes_n; i++) thing_render(box[i]);

		glfwSwapBuffers(window);
#endif
		number_of_frames++;






		time_last = time_current;
	}
	printf("A total of %d frames were rendered in %f time\n", number_of_frames, glfwGetTime() - time_start);

	for (i = 0; i < boxes_n; i++) thing_render(box[i]);
	glDeleteProgram(prog);
	glDeleteShader(frag);
	glDeleteShader(vert);
	end();
	return 0;
}

