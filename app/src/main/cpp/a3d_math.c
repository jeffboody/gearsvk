/*
 * Copyright (c) 2009-2010 Jeff Boody
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

/***********************************************************
* a3d - from https://github.com/jeffboody/a3d-native       *
***********************************************************/

#include "a3d_math.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "a3d_log.h"

void a3d_mat4f_copy(const a3d_mat4f_t* self, a3d_mat4f_t* copy)
{
	assert(self);
	assert(copy);

	copy->m00 = self->m00;
	copy->m01 = self->m01;
	copy->m02 = self->m02;
	copy->m03 = self->m03;
	copy->m10 = self->m10;
	copy->m11 = self->m11;
	copy->m12 = self->m12;
	copy->m13 = self->m13;
	copy->m20 = self->m20;
	copy->m21 = self->m21;
	copy->m22 = self->m22;
	copy->m23 = self->m23;
	copy->m30 = self->m30;
	copy->m31 = self->m31;
	copy->m32 = self->m32;
	copy->m33 = self->m33;
}

void a3d_mat4f_mulm_copy(const a3d_mat4f_t* self, const a3d_mat4f_t* m, a3d_mat4f_t* copy)
{
	assert(self);
	assert(m);
	assert(copy);

	copy->m00 = self->m00*m->m00 + self->m01*m->m10 + self->m02*m->m20 + self->m03*m->m30;
	copy->m01 = self->m00*m->m01 + self->m01*m->m11 + self->m02*m->m21 + self->m03*m->m31;
	copy->m02 = self->m00*m->m02 + self->m01*m->m12 + self->m02*m->m22 + self->m03*m->m32;
	copy->m03 = self->m00*m->m03 + self->m01*m->m13 + self->m02*m->m23 + self->m03*m->m33;
	copy->m10 = self->m10*m->m00 + self->m11*m->m10 + self->m12*m->m20 + self->m13*m->m30;
	copy->m11 = self->m10*m->m01 + self->m11*m->m11 + self->m12*m->m21 + self->m13*m->m31;
	copy->m12 = self->m10*m->m02 + self->m11*m->m12 + self->m12*m->m22 + self->m13*m->m32;
	copy->m13 = self->m10*m->m03 + self->m11*m->m13 + self->m12*m->m23 + self->m13*m->m33;
	copy->m20 = self->m20*m->m00 + self->m21*m->m10 + self->m22*m->m20 + self->m23*m->m30;
	copy->m21 = self->m20*m->m01 + self->m21*m->m11 + self->m22*m->m21 + self->m23*m->m31;
	copy->m22 = self->m20*m->m02 + self->m21*m->m12 + self->m22*m->m22 + self->m23*m->m32;
	copy->m23 = self->m20*m->m03 + self->m21*m->m13 + self->m22*m->m23 + self->m23*m->m33;
	copy->m30 = self->m30*m->m00 + self->m31*m->m10 + self->m32*m->m20 + self->m33*m->m30;
	copy->m31 = self->m30*m->m01 + self->m31*m->m11 + self->m32*m->m21 + self->m33*m->m31;
	copy->m32 = self->m30*m->m02 + self->m31*m->m12 + self->m32*m->m22 + self->m33*m->m32;
	copy->m33 = self->m30*m->m03 + self->m31*m->m13 + self->m32*m->m23 + self->m33*m->m33;
}

void a3d_mat4f_mulm(a3d_mat4f_t* self, const a3d_mat4f_t* m)
{
	assert(self);
	assert(m);

	a3d_mat4f_t copy;
	a3d_mat4f_mulm_copy(self, m, &copy);
	a3d_mat4f_copy(&copy, self);
}

void a3d_mat4f_ortho(a3d_mat4f_t* self, int load,
                     float l, float r,
                     float b, float t,
                     float n, float f)
{
	assert(self);

	if((l == r) || (t == b) || (n == f))
	{
		LOGE("invalid l=%f, r=%f, t=%f, b=%f, n=%f, f=%f", l, r, t, b, n, f);
		return;
	}

	float rml = r - l;
	float rpl = r + l;
	float tmb = t - b;
	float tpb = t + b;
	float fmn = f - n;
	float fpn = f + n;
	a3d_mat4f_t m =
	{
		2.0f/rml,     0.0f,      0.0f, 0.0f,
		    0.0f, 2.0f/tmb,      0.0f, 0.0f,
		    0.0f,     0.0f, -2.0f/fmn, 0.0f,
		-rpl/rml, -tpb/tmb,  -fpn/fmn, 1.0f,
	};

	if(load)
		a3d_mat4f_copy(&m, self);
	else
		a3d_mat4f_mulm(self, &m);
}

void a3d_mat4f_translate(a3d_mat4f_t* self, int load,
                         float x, float y, float z)
{
	assert(self);

	a3d_mat4f_t m =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		   x,    y,    z, 1.0f,
	};

	if(load)
		a3d_mat4f_copy(&m, self);
	else
		a3d_mat4f_mulm(self, &m);
}
