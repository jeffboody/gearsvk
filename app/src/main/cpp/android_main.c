/*
 * Copyright (c) 2019 Jeff Boody
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

#include <stdlib.h>
#include <assert.h>
#include <vulkan_wrapper.h>
#include <android_native_app_glue.h>
#include "gears_renderer.h"

#define LOG_TAG "gearsvk"
#include "a3d/a3d_log.h"

/***********************************************************
* platform                                                 *
***********************************************************/

typedef struct
{
	struct android_app* app;
	int focus;

	gears_renderer_t* renderer;
} platform_t;

static void onAppCmd(struct android_app* app, int32_t cmd);
static int32_t onInputEvent(struct android_app* app,
                            AInputEvent* event);

static platform_t*
platform_new(struct android_app* app)
{
	assert(app);

	platform_t* self = (platform_t*)
	                   calloc(1, sizeof(platform_t));
	if(self == NULL)
	{
		LOGE("calloc failed");
		return NULL;
	}

	self->app         = app;
	app->userData     = self;
	app->onAppCmd     = onAppCmd;
	app->onInputEvent = onInputEvent;

	return self;
}

static void
platform_termWindow(platform_t* self)
{
	assert(self);

	gears_renderer_delete(&self->renderer);
}

static void
platform_initWindow(platform_t* self)
{
	assert(self);

	if(self->renderer)
	{
		LOGW("renderer already exists");
		return;
	}

	LOGI("InitVulkan=%i", InitVulkan());

	uint32_t version = VK_MAKE_VERSION(1,0,0);
	self->renderer = gears_renderer_new(self->app,
	                                    "gearsvk",
	                                    version);
}

static void platform_delete(platform_t** _self)
{
	assert(_self);

	platform_t* self = *_self;
	if(self)
	{
		platform_termWindow(self);
		free(self);
		*_self = NULL;
	}
}

static void platform_draw(platform_t* self)
{
	assert(self);

	if(self->renderer)
	{
		gears_renderer_draw(self->renderer);
	}
}

static int platform_rendering(platform_t* self)
{
	assert(self);

	if(self->focus && self->renderer)
	{
		return 1;
	}

	return 0;
}

/***********************************************************
* callback functions                                       *
***********************************************************/

static void
onAppCmd(struct android_app* app, int32_t cmd)
{
	platform_t* platform = (platform_t*) app->userData;
	if(cmd == APP_CMD_INIT_WINDOW)
	{
		LOGI("APP_CMD_INIT_WINDOW");
		platform_initWindow(platform);
	}
	else if(cmd == APP_CMD_TERM_WINDOW)
	{
		LOGI("APP_CMD_TERM_WINDOW");
		platform_termWindow(platform);
	}
	else if(cmd == APP_CMD_GAINED_FOCUS)
	{
		LOGI("APP_CMD_GAINED_FOCUS");
		platform->focus = 1;
	}
	else if(cmd == APP_CMD_LOST_FOCUS)
	{
		LOGI("APP_CMD_LOST_FOCUS");
		platform_draw(platform);
		platform->focus = 0;
	}
	else
	{
		// ignore
	}
}

static int32_t
onInputEvent(struct android_app* app,
             AInputEvent* event)
{
	// ignore input events
	return 0;
}

/***********************************************************
* android_main                                             *
***********************************************************/

void android_main(struct android_app* app)
{
	platform_t* platform = platform_new(app);
	if(platform == NULL)
	{
		LOGE("platform failed");
		return;
	}

	while(1)
	{
		// poll for events
		int   id;
		int   outEvents;
		void* outData;
		id = ALooper_pollAll(platform_rendering(platform) ? 0 : -1,
		                     NULL, &outEvents, &outData);
		while(id > 0)
		{
			if((id == LOOPER_ID_MAIN) ||
			   (id == LOOPER_ID_INPUT))
			{
				struct android_poll_source* source;
				source = (struct android_poll_source*)
				         outData;
				if(source)
				{
					source->process(app, source);
				}
			}
			else if(id == LOOPER_ID_USER)
			{
				// ignore user events
			}

			// check for exit
			if(app->destroyRequested)
			{
				platform_delete(&platform);
				return;
			}

			id = ALooper_pollAll(platform_rendering(platform) ? 0 : -1,
		                         NULL, &outEvents, &outData);
		}

		// draw
		if(platform_rendering(platform))
		{
			platform_draw(platform);
		}
	}
}
