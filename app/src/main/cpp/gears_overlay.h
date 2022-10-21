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

typedef struct gears_renderer_s gears_renderer_t;

typedef struct gears_overlay_s
{
	vkk_uiScreen_t*    screen;
	vkk_engine_t*      engine;
	gears_viewAbout_t* view_about;
	gears_layerHud_t*  layer_hud;

	gears_renderer_t* renderer;
} gears_overlay_t;

gears_overlay_t* gears_overlay_new(vkk_engine_t* engine);
void             gears_overlay_delete(gears_overlay_t** _self);
void             gears_overlay_draw(gears_overlay_t* self);
void             gears_overlay_eventAction(gears_overlay_t* self,
                                           vkk_platformEvent_t* event);
void             gears_overlay_eventContentRect(gears_overlay_t* self,
                                                vkk_platformEventContentRect_t* ecr);
void             gears_overlay_eventDensity(gears_overlay_t* self,
                                            float density);
int              gears_overlay_eventKey(gears_overlay_t* self,
                                        vkk_platformEventKey_t* ek);
int              gears_overlay_event(gears_overlay_t* self,
                                     vkk_platformEvent_t* event);

#endif
