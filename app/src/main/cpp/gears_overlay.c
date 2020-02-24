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
#include <stdlib.h>

#define LOG_TAG "gears"
#include "libcc/cc_log.h"
#include "libcc/cc_memory.h"
#include "gears_overlay.h"
#include "gears_renderer.h"

/***********************************************************
* private                                                  *
***********************************************************/

static int clickBack(vkui_widget_t* widget,
                     void* priv, int state,
                     float x, float y)
{
	ASSERT(widget);
	ASSERT(priv);
	LOGD("debug x=%f, y=%f", x, y);

	gears_overlay_t* overlay = (gears_overlay_t*) priv;
	if(state == VKUI_WIDGET_POINTER_UP)
	{
		overlay->draw_mode = GEARS_OVERLAY_DRAWMODE_HUD;
		vkui_layer_clear(overlay->layer_show);
		vkui_layer_add(overlay->layer_show,
		               (vkui_widget_t*) overlay->layer_hud);
	}
	return 1;
}

/***********************************************************
* public                                                   *
***********************************************************/

gears_overlay_t* gears_overlay_new(struct gears_renderer_s* renderer)
{
	ASSERT(renderer);

	vkk_engine_t* engine = renderer->engine;

	cc_vec4f_t clear =
	{
		.r = 0.0f,
		.g = 0.0f,
		.b = 0.0f,
		.a = 0.0f
	};

	gears_overlay_t* self = (gears_overlay_t*)
	                        MALLOC(sizeof(gears_overlay_t));
	if(self == NULL)
	{
		return NULL;
	}
	self->renderer = renderer;

	self->screen = vkui_screen_new(engine,
	                               vkk_engine_renderer(engine),
	                               GEARS_RESOURCE,
	                               (void*) renderer,
	                               gears_renderer_playClick);
	if(self->screen == NULL)
	{
		LOGE("fail_screen");
		goto fail_screen;
	}

	vkui_widgetLayout_t layout_show =
	{
		.wrapx    = VKUI_WIDGET_WRAP_STRETCH_PARENT,
		.wrapy    = VKUI_WIDGET_WRAP_STRETCH_PARENT,
		.stretchx = 1.0f,
		.stretchy = 1.0f
	};

	self->layer_show = vkui_layer_new(self->screen, 0,
	                                  &layout_show, &clear);
	if(self->layer_show == NULL)
	{
		LOGE("fail_layer_show");
		goto fail_layer_show;
	}

	self->view_about = gears_viewAbout_new(self, clickBack);
	if(self->view_about == NULL)
	{
		LOGE("fail_view_about");
		goto fail_view_about;
	}

	self->layer_hud = gears_layerHud_new(self);
	if(self->layer_hud == NULL)
	{
		LOGE("fail_layer_hud");
		goto fail_layer_hud;
	}

	vkui_screen_top(self->screen,
	                (vkui_widget_t*) self->layer_show);
	self->draw_mode = GEARS_OVERLAY_DRAWMODE_HUD;
	vkui_layer_clear(self->layer_show);
	vkui_layer_add(self->layer_show,
	               (vkui_widget_t*) self->layer_hud);

	// success
	return self;

	// failure
	fail_layer_hud:
		gears_viewAbout_delete(&self->view_about);
	fail_view_about:
		vkui_layer_delete(&self->layer_show);
	fail_layer_show:
		vkui_screen_delete(&self->screen);
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
		vkui_screen_top(self->screen, NULL);
		vkui_layer_clear(self->layer_show);
		gears_layerHud_delete(&self->layer_hud);
		gears_viewAbout_delete(&self->view_about);
		vkui_layer_delete(&self->layer_show);
		vkui_screen_delete(&self->screen);
		FREE(self);
		*_self = NULL;
	}
}

int gears_overlay_escape(gears_overlay_t* self)
{
	ASSERT(self);

	if(self->draw_mode == GEARS_OVERLAY_DRAWMODE_ABOUT)
	{
		self->draw_mode = GEARS_OVERLAY_DRAWMODE_HUD;
		vkui_layer_clear(self->layer_show);
		vkui_layer_add(self->layer_show,
		               (vkui_widget_t*) self->layer_hud);
		return 1;
	}

	return 0;
}

void gears_overlay_draw(gears_overlay_t* self,
                        float density)
{
	ASSERT(self);

	vkui_screen_density(self->screen, density);
	vkui_screen_draw(self->screen);
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
	return vkui_screen_pointerDown(self->screen, x, y, t0);
}

int gears_overlay_pointerUp(gears_overlay_t* self,
                            float x, float y, double t0)
{
	ASSERT(self);
	return vkui_screen_pointerUp(self->screen, x, y, t0);
}

int gears_overlay_pointerMove(gears_overlay_t* self,
                              float x, float y, double t0)
{
	ASSERT(self);
	return vkui_screen_pointerMove(self->screen, x, y, t0);
}
