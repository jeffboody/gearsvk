/*
 * Copyright (c) 2020 Jeff Boody
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
 */

#include <stdlib.h>

#define LOG_TAG "gears"
#include "libcc/cc_log.h"
#include "libvkk/vkk_platform.h"
#include "gears_renderer.h"

/***********************************************************
* callbacks                                                *
***********************************************************/

void* gearsvk_onCreate(vkk_engine_t* engine)
{
	ASSERT(engine);

	return (void*) gears_renderer_new(engine);
}

void gearsvk_onDestroy(void** _priv)
{
	ASSERT(_priv);

	gears_renderer_delete((gears_renderer_t**) _priv);
}

void gearsvk_onDraw(void* priv)
{
	ASSERT(priv);

	gears_renderer_draw((gears_renderer_t*) priv);
}

void gearsvk_onPause(void* priv)
{
	ASSERT(priv);

	// ignore
}

void gearsvk_onEvent(void* priv, vkk_event_t* event)
{
	ASSERT(priv);
	ASSERT(event);

	gears_renderer_t* renderer = (gears_renderer_t*) priv;

	if(event->type == VKK_EVENT_TYPE_ACTION_DOWN)
	{
		gears_renderer_touch(renderer,
		                     GEARS_TOUCH_ACTION_DOWN,
		                     event->action.count,
		                     event->ts,
		                     event->action.coord[0].x,
		                     event->action.coord[0].y,
		                     event->action.coord[1].x,
		                     event->action.coord[1].y,
		                     event->action.coord[2].x,
		                     event->action.coord[2].y,
		                     event->action.coord[3].x,
		                     event->action.coord[3].y);
	}
	else if(event->type == VKK_EVENT_TYPE_ACTION_MOVE)
	{
		gears_renderer_touch(renderer,
		                     GEARS_TOUCH_ACTION_MOVE,
		                     event->action.count,
		                     event->ts,
		                     event->action.coord[0].x,
		                     event->action.coord[0].y,
		                     event->action.coord[1].x,
		                     event->action.coord[1].y,
		                     event->action.coord[2].x,
		                     event->action.coord[2].y,
		                     event->action.coord[3].x,
		                     event->action.coord[3].y);
	}
	else if(event->type == VKK_EVENT_TYPE_ACTION_UP)
	{
		gears_renderer_touch(renderer,
		                     GEARS_TOUCH_ACTION_UP,
		                     event->action.count,
		                     event->ts,
		                     event->action.coord[0].x,
		                     event->action.coord[0].y,
		                     event->action.coord[1].x,
		                     event->action.coord[1].y,
		                     event->action.coord[2].x,
		                     event->action.coord[2].y,
		                     event->action.coord[3].x,
		                     event->action.coord[3].y);
	}
	else if(event->type == VKK_EVENT_TYPE_DENSITY)
	{
		gears_renderer_density(renderer,
		                       event->density);
	}
	else if((event->type == VKK_EVENT_TYPE_KEY_UP) ||
	        ((event->type == VKK_EVENT_TYPE_KEY_DOWN) &&
	         (event->key.repeat)))
	{
		gears_renderer_keyPress(renderer,
		                        event->key.keycode,
		                        event->key.meta);
	}
	else if(event->type == VKK_EVENT_TYPE_CONTENT_RECT)
	{
		gears_renderer_contentRect(renderer,
		                           event->content_rect.t,
		                           event->content_rect.l,
		                           event->content_rect.b,
		                           event->content_rect.r);
	}
}

vkk_platformInfo_t VKK_PLATFORM_INFO =
{
	.app_name    = "GearsVK",
	.app_version =
	{
		.major = 1,
		.minor = 0,
		.patch = 14,
	},
	.app_dir     = "GearsVK",
	.onCreate    = gearsvk_onCreate,
	.onDestroy   = gearsvk_onDestroy,
	.onPause     = gearsvk_onPause,
	.onDraw      = gearsvk_onDraw,
	.onEvent     = gearsvk_onEvent,
};
