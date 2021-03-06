/*
A thing is a static mesh, I think.
*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

typedef struct
{
	float x, y, z, sx, sy, sz; // Location and scale of the thing in the world.
	mat4* loc;
	float* data;   // The data of the thing (in |(x, y, z, r, g, b)|).
	GLshort* ind;  // The index description of the thing.
	int n;      // |data| stores |n| values.
	int m;      // |ind| stores |m| values.
	GLuint data_vbo, ind_vbo, vao, prog;
	GLuint perspective_uni, rotate_camera_uni, center_camera_uni, place_in_world_uni,
		   light_intensity_uni, light_ambient_uni, light_pos_uni, light_attn_uni;
} thing;

void thing_read_data(thing *t, char* location)
{
	FILE* file = fopen(location, "r");
	assert(file);
	float x;
	int i, n, m; 

	fscanf(file, "%d", &n);
	t->data = malloc(sizeof(float)*(t->n = n*10));
	for (i = 0; i < t->n; i++) fscanf(file, "%f", &(t->data[i]));

	fscanf(file, "%d", &m);
	t->ind = malloc(sizeof(GLshort)*(t->m = m));
	for (i = 0; i < t->m; i++) fscanf(file, "%hd", &(t->ind[i]));

	fclose(file);
}

void thing_make_vbo(thing *t)
{ // Generates and sets up the VBO for thing |t|.
	glGenBuffers(1, &t->data_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, t->data_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*(t->n), t->data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &t->ind_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, t->ind_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLshort)*(t->m), t->ind, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void thing_make_vao(thing *t)
{ // Generates and sets up the VAO for thing |t|.
	glGenVertexArrays(1, &t->vao);
	glBindVertexArray(t->vao);
	glBindBuffer(GL_ARRAY_BUFFER, t->data_vbo);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	size_t offset = 3*sizeof(float);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10*sizeof(float), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 10*sizeof(float), (void*)(3*sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 10*sizeof(float), (void*)(7*sizeof(float)));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, t->ind_vbo);
	glBindVertexArray(0);
}

thing* create_thing(char* location)
{
	thing *r = malloc(sizeof(thing));
	r->loc = mat4_create(NULL);
	r->x = r->y = r->z = 0.0;
	r->sx = r->sy = r->sz = 1.0;

	thing_read_data(r, location);
	thing_make_vbo(r);
	thing_make_vao(r);

	return r;
}

void thing_move(thing* t, float x, float y, float z)
{
	t->x += x, t->y += y, t->z += z;
}

void thing_scale(thing* t, float sx, float sy, float sz)
{
	t->sx = sx, t->sy = sy, t->sz = sz;
}

float calc_f_scale(float fov)
{ // FOV is in radians.
    return 1.0f/tanf(fov/2.0);
}

void thing_set_program(thing* t, GLuint prog)
{ // TODO: make this universal for all things.
	t->prog = prog;
	t->perspective_uni = glGetUniformLocation(t->prog, "perspective");
	t->center_camera_uni = glGetUniformLocation(t->prog, "center_camera");
	t->rotate_camera_uni = glGetUniformLocation(t->prog, "rotate_camera");
	t->place_in_world_uni = glGetUniformLocation(t->prog, "place_in_world");
	t->light_intensity_uni = glGetUniformLocation(t->prog, "light_intensity");
	t->light_ambient_uni = glGetUniformLocation(t->prog, "light_ambient");
	t->light_pos_uni = glGetUniformLocation(t->prog, "light_pos");
	t->light_attn_uni = glGetUniformLocation(t->prog, "light_attn");
	//assert(t->perspective_uni != -1);
	//assert(t->rotate_camera_uni != -1);
	//assert(t->center_camera_uni != -1);
	//assert(t->place_in_world_uni != -1);
	//assert(t->light_intensity_uni != -1);
	//assert(t->light_ambient_uni != -1);
	//assert(t->light_pos_uni != -1);
	//assert(t->light_attn_uni != -1);

	float f_scale = calc_f_scale(M_PI/2), fz_near = 0.1, fz_far = 1000.0;
	int i;
	float perspective[16];

	for (i = 0; i < 16; i++) perspective[i] = 0.0;
	perspective[0] = f_scale*HEIGHT/WIDTH;
	perspective[5] = f_scale;
	perspective[10] = (fz_far + fz_near)/(fz_near - fz_far);
	perspective[11] = -1.0;
	perspective[14] = (2.0*fz_far*fz_near)/(fz_near - fz_far);

#if 0
	for (i = 0; i < 16; i++) perspective[i] = 0.0;
	perspective[0] = 1.0;
	perspective[5] = 1.0;
	perspective[10] = 1.0;
	perspective[15] = 1.0;
#endif


	glUseProgram(t->prog);
	glUniformMatrix4fv(t->perspective_uni, 1, GL_FALSE, perspective);

	glUniform4f(t->light_intensity_uni, 0.8, 0.8, 0.8, 1.0);
	glUniform4f(t->light_ambient_uni, 0.2, 0.2, 0.2, 1.0);
	glUniform1f(t->light_attn_uni, 0.1);

	glUseProgram(0);
}

void thing_render(thing* t)
{
	mat4_translate_scale(t->loc, t->x, t->y, t->z, t->sx, t->sy, t->sz);
	//mat4_print(t->loc);

	glUseProgram(t->prog);
	glBindVertexArray(t->vao);
	glUniformMatrix4fv(t->place_in_world_uni, 1, GL_TRUE, t->loc->m);
	glUniformMatrix4fv(t->center_camera_uni, 1, GL_TRUE, camera.t->m);
	glUniformMatrix4fv(t->rotate_camera_uni, 1, GL_TRUE, camera.r->m);
	if (camera.headlight) glUniform3f(t->light_pos_uni, camera.x, camera.y, camera.z);
	else glUniform3f(t->light_pos_uni, camera.hx, camera.hy, camera.hz);
	//glUniform3f(t->light_pos_uni, 0.0, 10.0, 0.0);
	glDrawElements(GL_TRIANGLES, t->m, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void delete_thing(thing* t)
{
	free(t->data);
	free(t->ind);
	mat4_delete(t->loc);
	free(t);
}
