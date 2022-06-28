/*
 * Copyright (c) 2018 Jeff Boody
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

#ifndef gears_overlay_H
#define gears_overlay_H

#include "gears_layerHud.h"
#include "gears_viewAbout.h"
#include "libvkk/vkk_ui.h"

struct gears_renderer_s;

typedef struct gears_overlay_s
{
	struct gears_renderer_s* renderer;

	vkk_uiScreen_t*    screen;
	gears_layerHud_t*  layer_hud;
	gears_viewAbout_t* view_about;
} gears_overlay_t;

gears_overlay_t* gears_overlay_new(struct gears_renderer_s* renderer);
void             gears_overlay_delete(gears_overlay_t** _self);
int              gears_overlay_escape(gears_overlay_t* self);
void             gears_overlay_draw(gears_overlay_t* self,
                                    float density);
void             gears_overlay_updateFps(gears_overlay_t* self,
                                         int fps);
int              gears_overlay_pointerDown(gears_overlay_t* self,
                                           float x, float y, double t0);
int              gears_overlay_pointerUp(gears_overlay_t* self,
                                         float x, float y, double t0);
int              gears_overlay_pointerMove(gears_overlay_t* self,
                                           float x, float y, double t0);
void             gears_overlay_contentRect(gears_overlay_t* self,
                                           uint32_t t, uint32_t l,
                                           uint32_t b, uint32_t r);

#endif
