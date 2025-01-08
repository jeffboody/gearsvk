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
#include "texgz/texgz_png.h"
#include "gears_overlay.h"
#include "gears_renderer.h"

/***********************************************************
* public                                                   *
***********************************************************/

static int
gears_overlay_screenshot(gears_overlay_t* self)
{
	ASSERT(self);

	texgz_tex_t* tex;
	tex = texgz_tex_new(2048, 2048,
	                    2048, 2048,
	                    TEXGZ_UNSIGNED_BYTE,
	                    TEXGZ_RGBA, NULL);
	if(tex == NULL)
	{
		return 0;
	}

	vkk_image_t* img;
	img = vkk_image_new(self->engine,
	                    2048, 2048, 1,
	                    VKK_IMAGE_FORMAT_RGBA8888,
	                    0, VKK_STAGE_FS, NULL);
	if(img == NULL)
	{
		goto fail_img;
	}

	vkk_renderer_t* rend;
	rend = vkk_renderer_newImage(self->engine, 2048, 2048,
	                             VKK_IMAGE_FORMAT_RGBA8888,
	                             VKK_RENDERER_MSAA_ENABLE);
	if(rend == NULL)
	{
		goto fail_rend;
	}

	gears_renderer_t* renderer;
	renderer = gears_renderer_new(self->engine, rend);
	if(renderer == NULL)
	{
		goto fail_renderer;
	}

	float clear_color[] =
	{
		0.0f, 0.0f, 0.0f, 0.0f,
	};

	if(vkk_renderer_beginImage(rend, VKK_RENDERER_MODE_DRAW,
	                           img, clear_color) == 0)
	{
		goto fail_begin;
	}

	gears_renderer_draw(renderer, 2048, 2048);
	vkk_renderer_end(rend);

	if(vkk_image_readPixels(img, tex->pixels) == 0)
	{
		goto fail_readpixels;
	}

	char fname[256];
	snprintf(fname, 256, "%s/screenshot.png",
	         vkk_engine_externalPath(self->engine));
	if(texgz_png_export(tex, fname) == 0)
	{
		goto fail_export;
	}

	gears_renderer_delete(&renderer);
	vkk_renderer_delete(&rend);
	vkk_image_delete(&img);
	texgz_tex_delete(&tex);

	// success
	return 1;

	// failure
	fail_export:
	fail_readpixels:
	fail_begin:
		gears_renderer_delete(&renderer);
	fail_renderer:
		vkk_renderer_delete(&rend);
	fail_rend:
		vkk_image_delete(&img);
	fail_img:
		texgz_tex_delete(&tex);
	return 0;
}

static int
gears_overlay_eventKey(gears_overlay_t* self,
                       vkk_platformEvent_t* event)
{
	ASSERT(self);
	ASSERT(event);

	vkk_platformEventKey_t* e = &event->key;

	if(e->keycode == '=')
	{
		if(event->type == VKK_PLATFORM_EVENTTYPE_KEY_UP)
		{
			gears_overlay_screenshot(self);
		}
	}
	else
	{
		return 0;
	}

	return 1;
}

/***********************************************************
* public                                                   *
***********************************************************/

gears_overlay_t*
gears_overlay_new(vkk_engine_t* engine)
{
	ASSERT(engine);

	vkk_renderer_t* rend = vkk_engine_defaultRenderer(engine);

	gears_overlay_t* self = (gears_overlay_t*)
	                        CALLOC(1, sizeof(gears_overlay_t));
	if(self == NULL)
	{
		return NULL;
	}
	self->engine = engine;

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

	self->screen = vkk_uiScreen_new(0, engine, rend, resource,
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

	self->renderer = gears_renderer_new(engine, rend);
	if(self->renderer == NULL)
	{
		goto fail_renderer;
	}

	vkk_uiScreen_windowReset(self->screen,
	                         (vkk_uiWindow_t*) self->layer_hud);

	// success
	return self;

	// failure
	fail_renderer:
		gears_layerHud_delete(&self->layer_hud);
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
		gears_renderer_delete(&self->renderer);
		gears_layerHud_delete(&self->layer_hud);
		gears_viewAbout_delete(&self->view_about);
		vkk_uiScreen_delete(&self->screen);
		FREE(self);
		*_self = NULL;
	}
}

void gears_overlay_draw(gears_overlay_t* self)
{
	ASSERT(self);

	vkk_renderer_t* rend;
	rend = vkk_engine_defaultRenderer(self->engine);

	float clear_color[4] =
	{
		0.0f, 0.0f, 0.0f, 1.0f
	};
	if(vkk_renderer_beginDefault(rend,
	                             VKK_RENDERER_MODE_DRAW,
	                             clear_color) == 0)
	{
		return;
	}
	vkk_uiScreen_draw(self->screen);
	vkk_renderer_end(rend);
}

int gears_overlay_event(gears_overlay_t* self,
                        vkk_platformEvent_t* event)
{
	ASSERT(self);
	ASSERT(event);

	if((event->type == VKK_PLATFORM_EVENTTYPE_ACTION_DOWN) ||
	   (event->type == VKK_PLATFORM_EVENTTYPE_ACTION_MOVE) ||
	   (event->type == VKK_PLATFORM_EVENTTYPE_ACTION_UP))
	{
		vkk_uiScreen_eventAction(self->screen, event);
	}
	else if(event->type == VKK_PLATFORM_EVENTTYPE_DENSITY)
	{
		vkk_uiScreen_eventDensity(self->screen, event->density);
	}
	else if((event->type == VKK_PLATFORM_EVENTTYPE_KEY_UP) ||
	        ((event->type == VKK_PLATFORM_EVENTTYPE_KEY_DOWN) &&
	         (event->key.repeat)))
	{
		if(gears_overlay_eventKey(self, event) == 0)
		{
			return vkk_uiScreen_eventKey(self->screen, &event->key);
		}
	}
	else if(event->type == VKK_PLATFORM_EVENTTYPE_CONTENT_RECT)
	{
		vkk_uiScreen_eventContentRect(self->screen,
		                              &event->content_rect);
	}

	return 1;
}
