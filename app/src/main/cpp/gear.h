/*
 * Copyright (c) 2009-2010 Jeff Boody
 *
 * Gears for Android is a heavily modified port of the famous "gears" demo.
 * As such, it is a derived work subject to the license requirements (below)
 * of the original work.
 *
 * Copyright (c) 1999-2001  Brian Paul   All Rights Reserved.
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

#ifndef gear_H
#define gear_H

#include "libcc/math/cc_mat4f.h"
#include "libcc/math/cc_vec4f.h"
#include "libvkk/vkk.h"

struct gears_renderer_s;

typedef struct
{
	struct gears_renderer_s* renderer;

	// color
	cc_vec4f_t color;

	// dynamic buffer data
	vkk_buffer_t* mvp_ub;
	vkk_buffer_t* nm_ub;

	// constant buffer data
	vkk_buffer_t* color_ub;

	// vertex data
	uint32_t      frontface_vc;
	vkk_buffer_t* frontface_vb;
	vkk_buffer_t* frontface_nb;
	uint32_t      backface_vc;
	vkk_buffer_t* backface_vb;
	vkk_buffer_t* backface_nb;
	uint32_t      outward_vc;
	vkk_buffer_t* outward_vb;
	vkk_buffer_t* outward_nb;
	uint32_t      cylinder_vc;
	vkk_buffer_t* cylinder_vb;
	vkk_buffer_t* cylinder_nb;

	// uniforms
	vkk_uniformSet_t* us;
} gear_t;

gear_t* gear_new(struct gears_renderer_s* renderer,
                 const cc_vec4f_t* color,
                 float inner_radius, float outer_radius,
                 float width,
                 int teeth, float tooth_depth);
void    gear_delete(gear_t** _self);
void    gear_update(gear_t* self,
                    cc_mat4f_t* mvp, cc_mat4f_t* mvm);
void    gear_draw(gear_t* self);

#endif
