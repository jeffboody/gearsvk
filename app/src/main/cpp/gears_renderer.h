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
#include "libcc/math/cc_stack4f.h"
#include "libvkk/vkk.h"
#include "gears_overlay.h"
#include "gear.h"

#define GEARS_TOUCH_ACTION_DOWN 0
#define GEARS_TOUCH_ACTION_UP   1
#define GEARS_TOUCH_ACTION_MOVE 2

#define GEARS_TOUCH_STATE_INIT    0
#define GEARS_TOUCH_STATE_ROTATE  1
#define GEARS_TOUCH_STATE_ZOOM    2
#define GEARS_TOUCH_STATE_OVERLAY 3

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

	// view state
	float           view_scale;
	cc_quaternion_t view_q;

	// animation state
	float angle;

	// matrices
	cc_mat4f_t    pm;
	cc_mat4f_t    mvm;
	cc_stack4f_t* mvm_stack;

	// fps state
	double t0;
	double t_last;
	int    frames;
	float  last_fps;

	// touch state
	int   touch_state;
	float touch_x1;
	float touch_y1;
	float touch_ds;

	// escape state
	double escape_t0;

	// overlay
	float            density;
	gears_overlay_t* overlay;

	// per-gear state
	gear_t* gear1;
	gear_t* gear2;
	gear_t* gear3;
} gears_renderer_t;

gears_renderer_t* gears_renderer_new(vkk_engine_t* engine);
void              gears_renderer_delete(gears_renderer_t** _self);
void              gears_renderer_exit(gears_renderer_t* self);
void              gears_renderer_loadURL(gears_renderer_t* self,
                                         const char* url);
void              gears_renderer_playClick(void* ptr);
int               gears_renderer_resize(gears_renderer_t* self);
int               gears_renderer_recreate(gears_renderer_t* self);
void              gears_renderer_density(gears_renderer_t* self,
                                         float density);
void              gears_renderer_draw(gears_renderer_t* self);
void              gears_renderer_touch(gears_renderer_t* self,
                                       int action, int count, double ts,
                                       float x0, float y0,
                                       float x1, float y1,
                                       float x2, float y2,
                                       float x3, float y3);
void              gears_renderer_keyPress(gears_renderer_t* self,
                                          int keycode, int meta);

#endif
