// maybe better to NOT reinvent the wheel
#include <math.h>









typedef struct
{
	float m[16];
} mat4;




void mat4_reset(mat4* m)
{ // Sets |m| to the identity matrix.
	int i;
	for (i = 0; i < 16; i++) m->m[i] = 0.0;
	m->m[0] = m->m[5] = m->m[10] = m->m[15] = 1.0;
}

mat4* mat4_create(float* m)
{ // Creates the 4x4 matrix
  //    m[0]  m[1]  m[2]  m[3]
  //    m[4]  m[5]  m[6]  m[7]
  //    m[7]  m[9]  m[10] m[11]
  //    m[12] m[13] m[14] m[15],
  // if |m| is not NULL.
  // If |m| is NULL the identity matrix is returned.
	mat4* r = malloc(sizeof(mat4));
	int i;
	if (!m)
	{
		mat4_reset(r);
		return r;
	}
	for (i = 0; i < 16; i++) r->m[i] = m[i];
	return r;
}

void mat4_delete(mat4* m)
{
	free(m);
}

void mat4_multiply(mat4* a, mat4* b)
{ // Sets |a| to the matrix multiple |a*b|.
	int i, j, k;
	float m[16];
	for (i = 0; i < 16; i++) m[i] = 0.0;
	for (i = 0; i < 4; i++) for (j = 0; j < 4; j++) for (k = 0; k < 4; k++) m[i*4 + j] += a->m[j*4 + k]*b->m[i*4 + k];
	for (i = 0; i < 16; i++) a->m[i] = m[i];
}

void mat4_translate_scale(mat4* a, float x, float y, float z, float sx, float sy, float sz)
{ // Sets |a| to the matrix that translates and scales each coordinate accordingly.
	int i;
	for (i = 0; i < 16; i++) a->m[i] = 0.0;
	a->m[3] = x, a->m[7] = y, a->m[11] = z;
	a->m[0] = sx, a->m[5] = sy, a->m[10] = sz, a->m[15] = 1.0;
}

/*
|   1   0   0   0 ||  cp   0   sp   0 |
|   0  ct -st   0 ||   0   1    0   0 |
|   0  st  ct   0 || -sp   0   cp   0 |
|   0   0   0   1 ||   0   0    0   1 |
=
|     cp  0     sp 0 |
|  st*sp ct -st*cp 0 |
| -ct*sp st  ct*cp 0 |
|      0  0      0 1 |
 */
void mat4_camera_rotation(mat4* a, float theta, float phi)
{ // This sets |a| to a matrix that can used to rotate the camera.
  // First there is a horizontal rotation, and then a vertical rotation.
	int i;
	for (i = 0; i < 16; i++) a->m[i] = 0.0;
	float st = sinf(theta), ct = cosf(theta), sp = sinf(phi), cp = cosf(phi);
	a->m[0] = cp;
	a->m[1] = 0.0;
	a->m[2] = sp;
	a->m[3] = 0.0;

	a->m[4] = st*sp;
	a->m[5] = ct;
	a->m[6] = -st*cp;
	a->m[7] = 0.0;

	a->m[8] = -ct*sp;
	a->m[9] = st;
	a->m[10] = ct*cp;
	a->m[11] = 0.0;

	a->m[12] = 0.0;
	a->m[13] = 0.0;
	a->m[14] = 0.0;
	a->m[15] = 1.0;
}

// wholly untested.
void mat4_rotate(mat4* a, float x, float y, float z, float theta)
{ // Sets |a| to the matrix that rotates around the vector (x, y, z).
  // Notable examples of vectors are (1, 0, 0), (0, 1, 0), and (0, 0, 1).
	int i;
	for (i = 0; i < 16; i++) a->m[i] = 0.0;
	float s = sinf(theta), c = cosf(theta), cc = 1 - c;
	a->m[0] = x*x + (1.0 - x*x)*c;
	a->m[1] = cc*x*y - z*s;
	a->m[2] = cc*x*z + y*s;
	a->m[3] = 0.0;

	a->m[4] = cc*x*y + z*s;
	a->m[5] = y*y + (1.0 - y*y)*c;
	a->m[6] = cc*y*z - x*s;
	a->m[7] = 0.0;

	a->m[8] = cc*x*z - y*s;
	a->m[9] = cc*y*z + x*s;
	a->m[10] = z*z + (1.0 - z*z)*c;
	a->m[11] = 0.0;

	a->m[12] = 0.0;
	a->m[13] = 0.0;
	a->m[14] = 0.0;
	a->m[15] = 1.0;
}



void mat4_print(mat4* m)
{
	printf("printing mat4:\n");
	printf("%8.3f %8.3f %8.3f %8.3f\n", m->m[0], m->m[1], m->m[2], m->m[3]);
	printf("%8.3f %8.3f %8.3f %8.3f\n", m->m[4], m->m[5], m->m[6], m->m[7]);
	printf("%8.3f %8.3f %8.3f %8.3f\n", m->m[8], m->m[9], m->m[10], m->m[11]);
	printf("%8.3f %8.3f %8.3f %8.3f\n", m->m[12], m->m[13], m->m[14], m->m[15]);
	printf("done\n");
}

