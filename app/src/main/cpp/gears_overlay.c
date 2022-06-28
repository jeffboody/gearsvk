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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define LOG_TAG "gears"
#include "libcc/cc_log.h"
#include "libcc/cc_memory.h"
#include "gears_overlay.h"
#include "gears_renderer.h"

/***********************************************************
* public                                                   *
***********************************************************/

gears_overlay_t*
gears_overlay_new(struct gears_renderer_s* renderer)
{
	ASSERT(renderer);

	vkk_engine_t* engine = renderer->engine;

	gears_overlay_t* self = (gears_overlay_t*)
	                        CALLOC(1, sizeof(gears_overlay_t));
	if(self == NULL)
	{
		return NULL;
	}
	self->renderer = renderer;

	const char* resource_path;
	resource_path = vkk_engine_internalPath(engine);

	char resource[256];
	snprintf(resource, 256, "%s/resource.bfs", resource_path);

	vkk_uiWidgetStyle_t widget_style =
	{
		.color_primary =
		{
			.r=0.306f,
			.g=0.8f,
			.b=0.639f,
			.a=1.0f
		},
		.color_secondary =
		{
			.r=0.224f,
			.g=0.243f,
			.b=0.275f,
			.a=1.0f
		},
		.color_text =
		{
			.r=1.0f,
			.g=1.0f,
			.b=1.0f,
			.a=1.0f
		},
		.color_background =
		{
			.r=0.122f,
			.g=0.122f,
			.b=0.122f,
			.a=1.0f
		},
	};

	self->screen = vkk_uiScreen_new(0, engine,
	                                vkk_engine_defaultRenderer(engine),
	                                resource,
	                                (void*) renderer,
	                                gears_renderer_playClick,
	                                &widget_style);
	if(self->screen == NULL)
	{
		goto fail_screen;
	}

	self->view_about = gears_viewAbout_new(self);
	if(self->view_about == NULL)
	{
		goto fail_view_about;
	}

	self->layer_hud = gears_layerHud_new(self);
	if(self->layer_hud == NULL)
	{
		goto fail_layer_hud;
	}

	vkk_uiScreen_windowReset(self->screen,
	                         (vkk_uiWindow_t*) self->layer_hud);

	// success
	return self;

	// failure
	fail_layer_hud:
		gears_viewAbout_delete(&self->view_about);
	fail_view_about:
		vkk_uiScreen_delete(&self->screen);
	fail_screen:
		FREE(self);
	return NULL;
}

void gears_overlay_delete(gears_overlay_t** _self)
{
	ASSERT(_self);

	gears_overlay_t* self = *_self;
	if(self)
	{
		vkk_uiScreen_windowReset(self->screen, NULL);
		gears_layerHud_delete(&self->layer_hud);
		gears_viewAbout_delete(&self->view_about);
		vkk_uiScreen_delete(&self->screen);
		FREE(self);
		*_self = NULL;
	}
}

int gears_overlay_escape(gears_overlay_t* self)
{
	ASSERT(self);

	return vkk_uiScreen_windowPop(self->screen);
}

void gears_overlay_draw(gears_overlay_t* self,
                        float density)
{
	ASSERT(self);

	vkk_uiScreen_density(self->screen, density);
	vkk_uiScreen_draw(self->screen);
}

void gears_overlay_updateFps(gears_overlay_t* self, int fps)
{
	ASSERT(self);

	gears_layerHud_updateFps(self->layer_hud, fps);
}

int gears_overlay_pointerDown(gears_overlay_t* self,
                              float x, float y, double t0)
{
	ASSERT(self);
	return vkk_uiScreen_pointerDown(self->screen, x, y, t0);
}

int gears_overlay_pointerUp(gears_overlay_t* self,
                            float x, float y, double t0)
{
	ASSERT(self);
	return vkk_uiScreen_pointerUp(self->screen, x, y, t0);
}

int gears_overlay_pointerMove(gears_overlay_t* self,
                              float x, float y, double t0)
{
	ASSERT(self);
	return vkk_uiScreen_pointerMove(self->screen, x, y, t0);
}

void gears_overlay_contentRect(gears_overlay_t* self,
                               uint32_t t, uint32_t l,
                               uint32_t b, uint32_t r)
{
	ASSERT(self);

	vkk_uiScreen_contentRect(self->screen, t, l, b, r);
}
