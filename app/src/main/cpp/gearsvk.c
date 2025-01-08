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
#include "gears_overlay.h"

/***********************************************************
* callbacks                                                *
***********************************************************/

void* gearsvk_onCreate(vkk_engine_t* engine)
{
	ASSERT(engine);

	return (void*) gears_overlay_new(engine);
}

void gearsvk_onDestroy(void** _priv)
{
	ASSERT(_priv);

	gears_overlay_delete((gears_overlay_t**) _priv);
}

void gearsvk_onDraw(void* priv)
{
	ASSERT(priv);

	gears_overlay_draw((gears_overlay_t*) priv);
}

void gearsvk_onPause(void* priv)
{
	ASSERT(priv);

	// ignore
}

int gearsvk_onEvent(void* priv, vkk_platformEvent_t* event)
{
	ASSERT(priv);
	ASSERT(event);

	return gears_overlay_event((gears_overlay_t*) priv,
	                           event);
}

vkk_platformInfo_t VKK_PLATFORM_INFO =
{
	.app_name    = "GearsVK",
	.app_version =
	{
		.major = 1,
		.minor = 0,
		.patch = 24,
	},
	.app_dir     = "GearsVK",
	.onCreate    = gearsvk_onCreate,
	.onDestroy   = gearsvk_onDestroy,
	.onPause     = gearsvk_onPause,
	.onDraw      = gearsvk_onDraw,
	.onEvent     = gearsvk_onEvent,
};
