/*
A thing is a static mesh, I think.
*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

typedef struct
{
	float x, y, z; // Location of the thing in the world.
	float* data;   // The data of the thing (in |(x, y, z, r, g, b)|).
	GLshort* ind;  // The index description of the thing.
	int n;      // |data| stores |n| values.
	int m;      // |ind| stores |m| values.
	GLuint data_vbo, ind_vbo, vao, prog;
	GLuint offset_uni, perspective_uni;
} thing;

void thing_read_data(thing *t, char* location)
{
	FILE* file = fopen(location, "r");
	assert(file);
	float x;
	int i, n, m; 

	fscanf(file, "%d", &n);
	t->data = malloc(sizeof(float)*(t->n = n*7));
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
	size_t offset = 3*sizeof(float);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)offset);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, t->ind_vbo);
	glBindVertexArray(0);
}

thing* create_thing(char* location)
{
	thing *r = malloc(sizeof(thing));
	r->x = r->y = r->z = 0.0;

	thing_read_data(r, location);
	thing_make_vbo(r);
	thing_make_vao(r);

	return r;
}

void thing_move(thing* t, float x, float y, float z)
{
	t->x += x, t->y += y, t->z += z;
}

void thing_set_program(thing* t, GLuint prog)
{ // TODO: make this universal for all things.
	t->prog = prog;
	t->offset_uni = glGetUniformLocation(t->prog, "offset");
	t->perspective_uni = glGetUniformLocation(t->prog, "perspective");
	assert(t->offset_uni != -1);
	//assert(t->perspective_uni != -1);

	float fz_near = 1.0, fz_far = 1000.0, f_scale = 1.0;
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
	glUseProgram(0);
}

void thing_render(thing* t)
{
	glUseProgram(t->prog);

	glBindVertexArray(t->vao);
	glUniform3f(t->offset_uni, t->x, t->y, t->z);
	glDrawElements(GL_TRIANGLES, t->m, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void delete_thing(thing* t)
{
	free(t->data);
	free(t->ind);
	free(t);
}
