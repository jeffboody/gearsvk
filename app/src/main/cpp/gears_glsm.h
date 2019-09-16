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

#ifndef gears_glsm_H
#define gears_glsm_H

#include <stdint.h>

#include "libcc/math/cc_vec3f.h"
#include "libcc/cc_list.h"

#define GEARS_GLSM_COMPLETE   0
#define GEARS_GLSM_INCOMPLETE 1
#define GEARS_GLSM_ERROR      2

// "glsm" - gl state machine
typedef struct
{
	// state
	int status;
	cc_vec3f_t normal;
	cc_list_t* cache_vb;   // vertex(s)
	cc_list_t* cache_nb;   // normal(s)

	// "completed" arrays
	uint32_t ec;   // element count
	float*   vb;   // vertex(s)
	float*   nb;   // normal(s)
} gears_glsm_t;

gears_glsm_t* gears_glsm_new(void);
void          gears_glsm_delete(gears_glsm_t** _self);
void          gears_glsm_begin(gears_glsm_t* self);
void          gears_glsm_normal3f(gears_glsm_t* self, float x, float y, float z);
void          gears_glsm_vertex3f(gears_glsm_t* self, float x, float y, float z);
void          gears_glsm_end(gears_glsm_t* self);
int           gears_glsm_status(gears_glsm_t* self);

#endif
