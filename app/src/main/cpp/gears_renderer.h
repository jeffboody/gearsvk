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

#ifndef gears_renderer_H
#define gears_renderer_H

#include "libcc/math/cc_quaternion.h"
#include "libcc/math/cc_mat4f.h"
#include "libcc/math/cc_rect12f.h"
#include "libcc/math/cc_stack4f.h"
#include "libvkk/vkk.h"
#include "gear.h"

/***********************************************************
* public                                                   *
***********************************************************/

typedef struct gears_renderer_s
{
	vkk_engine_t*            engine;
	vkk_uniformSetFactory_t* usf;
	vkk_pipelineLayout_t*    pl;
	vkk_graphicsPipeline_t*  gp;
	vkk_image_t*             image;

	// state
	float           draw_w;
	float           draw_h;
	float           view_scale;
	cc_quaternion_t view_q;
	float           angle;

	// matrices
	cc_mat4f_t    pm;
	cc_mat4f_t    mvm;
	cc_stack4f_t* mvm_stack;

	// per-gear state
	gear_t* gear1;
	gear_t* gear2;
	gear_t* gear3;
} gears_renderer_t;

gears_renderer_t* gears_renderer_new(vkk_engine_t* engine);
void              gears_renderer_delete(gears_renderer_t** _self);
void              gears_renderer_rotate(gears_renderer_t* self,
                                        float dx, float dy);
void              gears_renderer_scale(gears_renderer_t* self,
                                       float scale);
void              gears_renderer_draw(gears_renderer_t* self,
                                      float draw_w,
                                      float draw_h);

#endif
