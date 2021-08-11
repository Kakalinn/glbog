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


typedef float vec3[3];

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

#ifdef MOUSE_MOVEMENT
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (glfwRawMouseMotionSupported()) glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
#endif

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
	camera.theta = clampf(camera.theta, -M_PI/2.0, M_PI/2.0);
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
	if (y < yl) return 0;
	printf("4\n");
	if (y > yr) return 0;
	printf("5\n");
	if (z < zl - tol) return 0;
	printf("6\n");
	if (z > zr + tol) return 0;
	printf("7\n");

	if (fabsf(x - xl) <= tol) return 1;
	if (fabsf(x - xr) <= tol) return 1;
	return 2;
}

//c -s | x
//s  c | y
//
//x*c - s*y
//x*s + c*y

float collision_tol = 0.001;
#if 0
int line_segment_collision_check(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float* x, float* y, float* t)
{ // Essentially, does the linesegment <(x1, y1), (x2, y2)> intersect with <(x3, y3), (x4, y4)>.
	// And if so, where?
	// The player is going from point (x3, y3) to (x4, y4).
	x2 -= x1, y2 -= y1, x3 -= x1, y3 -= y1, x4 -= x1, y4 -= y1;
	float l = hypotf(x2, y2);
	float c = x2/l, s = -y2/l;
	float x5 = x3*c - y3*s, y5 - x3*s + y3*c,
		  x6 = x4*c - y4*s, y6 - x4*s + y4*c;
	if (fabsf(y5) < collision_tol)
	{
		*x = x3, *y = y5;
		return 1;
	}
	if (y5 < 0.0 && y6 < 0.0) return 0;
	if (y5 > 0.0 && y6 > 0.0) return 0;
	*t = y5/(y5 + y6);
	float tx = x6*(*t) + x5*(1.0 - *t);
	if (tx < -collision_tolerance) return 0;
	if (tx > l + collision_tolerance) return 0;
	*x = x4*t + x3*(1.0 - t), *y = y4*t + y3*(1.0 - t);
	return 1;
}

int plane_collision_check(float x, float y, float z, float xv, float yv, float zv, float px1, float py1, float pz1, float px2, float py2, float pz2)
{ // The player is at (x, y, z) with velocity vector (xv, yv, zv).
	// The velocity vector is distance travlled in the next tick.
	// The collision plane is ...
	if (y > py2 + collision_tolerance) return 0; // above collision.
	if (y < py1 - collision_tolerance) return 0; // below collision.

}
#endif

int line_plane_intersection(
		float xl, float yl, float zl, float xv, float yv, float zv,
		float xp, float yp, float zp, float xn, float yn, float zn, float* t)
{ // Line through (xl, yl, zl) in direction (xv, yv, zv).
	// Plane through (xp, yp, zp) with normal (xn, yn, zn).
	// They intersect at (xl, yl, zl) + t*(xv, xy, zy) if it retruns 1.
	// There is not precisly one intersection if it returns 0.
	float d = (xl*xn + yl*yn + zl*zn);
	if (fabsf(d) < collision_tol) return 0;
	*t = (xn*(xp - xl) + yn*(yp - yl) + zn*(zp - zl))/d;
}

void vector_clamping(float* x, float* y, float px, float py)
{ // (x, y), and (px, py) are all vectors.
	// (px, py) needs to be normal.
	// All of (x, y) pointing in the direction of (px, py) is removed).
	float d = *x*px + *y*py;
	*x -= px*d, *y -= py*d;
}

float vec3_dot(vec3 a, vec3 b)
{
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

void vec3_scale(vec3 v, float s)
{
	v[0] *= s, v[1] *= s, v[2] *= s;
}

void vec3_normalize(vec3 v)
{
	float l = sqrt(vec3_dot(v, v));
	if (l < 0.0001) return;
	v[0] /= l, v[1] /= l, v[2] /= l;
}


vec3 vel = {0.0, 0.0, 0.0};

void friction(float friction, float time)
{
	float low_enough = 1.0, speed = sqrt(vec3_dot(vel, vel)), drop = 0, control, newspeed, stopspeed = 0.01;

	if (speed < low_enough)
	{
		vel[0] = vel[2] = 0.0;
		return;
	}

	control = speed < stopspeed ? stopspeed : speed;
	drop += control*friction*time;

	newspeed = speed - drop;
	if (newspeed < 0) newspeed = 0;
	newspeed /= speed;

	vel[0] = vel[0] * newspeed;
	//vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}

float max_air_speed, max_speed, max_accel;
void accelerate(vec3 wishdir, float time)
{
	int i;
	float addspeed = clampf(max_speed - vec3_dot(vel, wishdir), 0.0, max_accel*time);
	printf(">> addspeed %f\n", addspeed);
	for (i = 0; i < 3; i++) vel[i] += addspeed*wishdir[i];
}

void air_accelerate(vec3 wishdir, float time)
{
	int i;
	float addspeed = clampf(max_air_speed - vec3_dot(vel, wishdir), 0.0, max_accel*time);
	printf(">> addspeed %f\n", addspeed);
	for (i = 0; i < 3; i++) vel[i] += addspeed*wishdir[i];
}



int main()
{
	max_air_speed = 0.8, max_speed = 8.0, max_accel = 10.0*max_speed;
	init();
	GLuint vert, frag, prog;
	int i, j, boxes_n = 3, grounded = 0;
	float grav = -10.0, ymaxvel = 0.1;
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
	thing_move(box[1], -0.2, -1.1, -3.0);
	thing_move(box[2], -0.2, -1.1, -4.0);

	thing_scale(box[0], 200.0, 1.0, 200.0);
	thing_scale(box[1], 0.5, 0.5, 0.5);
	thing_scale(box[2], 1.0, 1.0, 1.0);

	double time_current, time_elapsed, time_start = glfwGetTime(), time_last = glfwGetTime();
	int number_of_frames = 0;
#ifdef MOUSE_MOVEMENT
	double mouse_x, mouse_y, sensitivity = 0.1;
	glfwGetCursorPos(window, &mouse_x, &mouse_y);
#endif
	int is_first_ground_frame = 1;
	while (!glfwWindowShouldClose(window))
	{
		printf(">> xz-plane velocity: %f\n", hypot(vel[0], vel[2]));
		printf(">>>>>> %f %f %f (%f %f %f)\n", camera.x, camera.y, camera.z, vel[0], vel[1], vel[2]);
		glfwPollEvents();

		time_current = glfwGetTime();
		time_elapsed = time_current - time_last;

		// handle inputs
		float xx = 0.0, yy = 0.0, zz = 0.0, tt = 0.0, pp = 0.0, ss = sin(camera.phi), cc = cos(camera.phi);
		vec3 wishdir = {0.0, 0.0, 0.0};
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;
		if (glfwGetKey(window, GLFW_KEY_W)      == GLFW_PRESS) wishdir[0] += -ss, wishdir[2] += -cc;
		if (glfwGetKey(window, GLFW_KEY_A)      == GLFW_PRESS) wishdir[0] += -cc, wishdir[2] +=  ss;
		if (glfwGetKey(window, GLFW_KEY_S)      == GLFW_PRESS) wishdir[0] +=  ss, wishdir[2] +=  cc;
		if (glfwGetKey(window, GLFW_KEY_D)      == GLFW_PRESS) wishdir[0] +=  cc, wishdir[2] += -ss;
		printf(">> 1 %f %f %f (%f)\n", wishdir[0], wishdir[1], wishdir[2], vec3_dot(wishdir, wishdir));
		vec3_normalize(wishdir);
		//vec3_scale(wishdir, 360.0);
		printf(">> 2 %f %f %f (%f)\n", wishdir[0], wishdir[1], wishdir[2], vec3_dot(wishdir, wishdir));
		printf("\n\n");

		if (glfwGetKey(window, GLFW_KEY_RIGHT)  == GLFW_PRESS) pp += -2.0;
		if (glfwGetKey(window, GLFW_KEY_LEFT)   == GLFW_PRESS) pp +=  2.0;
		if (glfwGetKey(window, GLFW_KEY_DOWN)   == GLFW_PRESS) tt += -2.0;
		if (glfwGetKey(window, GLFW_KEY_UP)     == GLFW_PRESS) tt +=  2.0;

#ifdef MOUSE_MOVEMENT
		double mouse_xx, mouse_yy;
		glfwGetCursorPos(window, &mouse_xx, &mouse_yy);
		pp += (mouse_x - mouse_xx)*sensitivity;
		tt += (mouse_y - mouse_yy)*sensitivity;
		mouse_x = mouse_xx, mouse_y = mouse_yy;
#endif


		// move player
		if (grounded)
		{
			if (is_first_ground_frame) is_first_ground_frame = 0;
			else friction(6.0, time_elapsed);

			accelerate(wishdir, time_elapsed);
			for (i = 0; i < boxes_n; i++) if (player_is_on_top_of_box(box[i])) break;
			if (i == boxes_n) grounded = 0;
			if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) vel[1] = 3.0, grounded = 0;
		}
		else
		{
			is_first_ground_frame = 1;
			air_accelerate(wishdir, time_elapsed);
			vel[1] += clampf(time_elapsed*grav, -ymaxvel, ymaxvel);

			if (vel[1] < 0.0)
			{
				for (i = 0; i < boxes_n; i++) if (player_is_on_top_of_box(box[i]))
				{
					vel[1] = 0.0;
					camera.y = box[i]->y + box[i]->sy + 0.1;
					grounded = 1;
					break;
				}
			}
			if (i < boxes_n) printf(">> landed on box[%d]\n", i);

			yy = vel[1];
		}

		//collisions with the boxes, simple
		float movement_time = time_elapsed;
#if 0
		for (i = 0; i < boxes_n; i++)
		{
			float t;
			if (line_plane_intersection(camera.x, camera.y, camera.z, xvel, yvel, zvel, box[i].x, box[i].y, box[i].z, 1.0, 0.0, 0.0, &t))
			{
				movement_time = fminf(movement_time, t);
			}


			printf("collinding with box[%d] of the %c variety (%d)\n", i, (j == 1 ? 'x' : 'z'), j);
			if (j == 1) xx = 0.0;
			else zz = 0.0;
		}
#endif


		move_camera(vel[0]*movement_time, vel[1]*movement_time, vel[2]*movement_time, tt*time_elapsed, pp*time_elapsed);


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

