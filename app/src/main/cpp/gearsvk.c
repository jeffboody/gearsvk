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

#include <assert.h>
#include <stdlib.h>

#define LOG_TAG "gears"
#include "libcc/cc_log.h"
#include "libvkk/vkk.h"
#include "gears_renderer.h"

/***********************************************************
* callbacks                                                *
***********************************************************/

void* gearsvk_onCreate(vkk_platform_t* platform)
{
	assert(platform);

	return (void*) gears_renderer_new(platform);
}

void gearsvk_onDestroy(void** _priv)
{
	assert(_priv);

	gears_renderer_delete((gears_renderer_t**) _priv);
}

void gearsvk_onDraw(void* priv)
{
	assert(priv);

	gears_renderer_draw((gears_renderer_t*) priv);
}

void gearsvk_onEvent(void* priv, vkk_event_t* event)
{
	assert(priv);
	assert(event);

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
	else if(event->type == VKK_EVENT_TYPE_RESIZE)
	{
		gears_renderer_resize(renderer);
	}
}

vkk_platformCallbacks_t VKK_PLATFORM_CALLBACKS =
{
	.onCreate  = gearsvk_onCreate,
	.onDestroy = gearsvk_onDestroy,
	.onDraw    = gearsvk_onDraw,
	.onEvent   = gearsvk_onEvent,
};
