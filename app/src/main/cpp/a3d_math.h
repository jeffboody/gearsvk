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

#ifndef a3d_math_H
#define a3d_math_H

typedef struct
{
	float m00;
	float m10;
	float m20;
	float m30;
	float m01;
	float m11;
	float m21;
	float m31;
	float m02;
	float m12;
	float m22;
	float m32;
	float m03;
	float m13;
	float m23;
	float m33;
} a3d_mat4f_t;

typedef struct
{
	float x;
	float y;
	float z;
} a3d_vec3f_t;

void a3d_mat4f_copy(const a3d_mat4f_t* self,
                    a3d_mat4f_t* copy);
void a3d_mat4f_mulm_copy(const a3d_mat4f_t* self,
                         const a3d_mat4f_t* m,
                         a3d_mat4f_t* copy);
void a3d_mat4f_mulm(a3d_mat4f_t* self,
                    const a3d_mat4f_t* m);
void a3d_mat4f_ortho(a3d_mat4f_t* self, int load,
                     float l, float r,
                     float b, float t,
                     float n, float f);
void a3d_mat4f_translate(a3d_mat4f_t* self, int load,
                         float x, float y, float z);

#endif
