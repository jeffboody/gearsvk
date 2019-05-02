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
#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "gearsvk", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "gearsvk", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "gearsvk", __VA_ARGS__))

/***********************************************************
* platform                                                 *
***********************************************************/

typedef struct
{
	struct android_app* app;
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	int focus;
} platform_t;

static void onAppCmd(struct android_app* app, int32_t cmd);
static int32_t onInputEvent(struct android_app* app,
                            AInputEvent* event);

static platform_t*
platform_new(struct android_app* app)
{
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
	if(self->display != EGL_NO_DISPLAY)
	{
		eglMakeCurrent(self->display,
		               EGL_NO_SURFACE, EGL_NO_SURFACE,
		               EGL_NO_CONTEXT);
		if(self->context != EGL_NO_CONTEXT)
		{
			eglDestroyContext(self->display, self->context);
			self->context = EGL_NO_CONTEXT;
		}

		if(self->surface != EGL_NO_SURFACE)
		{
			eglDestroySurface(self->display, self->surface);
			self->surface = EGL_NO_SURFACE;
		}

		eglTerminate(self->display);
		self->display = EGL_NO_DISPLAY;
	}
}

static void
platform_initWindow(platform_t* self)
{
	assert(self);

	if(self->display != EGL_NO_DISPLAY)
	{
		LOGW("initWindow called with existing display");
		engine_termWindow(self);
	}

	self->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if(self->display == EGL_NO_DISPLAY)
	{
		LOGE("eglGetDisplay failed");
		return;
	}

	EGLint major;
	EGLint minor;
	if(eglInitialize(self->display, &major, &minor) == EGL_FALSE)
	{
		LOGE("eglInitialize failed");
		goto fail_initalize;
	}
	LOGI("EGL %i.%i", major, minor);

	const EGLint attribs[] =
	{
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};

	// query the number of EGL configs
	EGLint num_config = 0;
	if(eglChooseConfig(self->display, attribs, NULL,
	                   0, &num_config) == EGL_FALSE)
	{
		LOGE("eglChooseConfig failed");
		goto fail_num_config;
	}

	// allocate temporary array of configs
	EGLConfig* configs;
	configs = (EGLConfig*)
	          calloc(num_config, sizeof(EGLConfig));
	if(configs == 0)
	{
		LOGE("calloc failed");
		goto fail_alloc_config;
	}

	// get the configs
	if(eglChooseConfig(self->display, attribs, configs,
	                   num_config, &num_config) == EGL_FALSE)
	{
		LOGE("eglChooseConfig failed");
		goto fail_get_config;
	}

	// choose a config
	int i;
	EGLConfig config;
	for(i = 0; i < num_config; ++i)
	{
		EGLint r, g, b;
		if(eglGetConfigAttrib(self->display, configs[i],
		                      EGL_RED_SIZE, &r)   &&
		   eglGetConfigAttrib(self->display, configs[i],
		                      EGL_GREEN_SIZE, &g) &&
		   eglGetConfigAttrib(self->display, configs[i],
		                      EGL_BLUE_SIZE, &b)  &&
		   (r == 8) && (g == 8) && (b == 8))
		{
			config = configs[i];
			break;
		}
	}

	if(i == num_config)
	{
		LOGE("invalid config");
		goto fail_choose;
	}

	self->surface = eglCreateWindowSurface(self->display,
	                                       config,
	                                       self->app->window,
	                                       NULL);
	if(self->surface == EGL_NO_SURFACE)
	{
		LOGE("eglCreateWindowSurface failed");
		goto fail_surface;
	}

	self->context = eglCreateContext(self->display, config,
	                                 NULL, NULL);
	if(self->context == EGL_NO_CONTEXT)
	{
		LOGE("eglCreateContext failed");
		goto fail_context;
	}

	if(eglMakeCurrent(self->display,
	                  self->surface, self->surface,
	                  self->context) == EGL_FALSE)
	{
		LOGE("eglMakeCurrent failed");
		goto fail_current;
	}

	eglQuerySurface(self->display, self->surface,
	                EGL_WIDTH, &self->width);
	eglQuerySurface(self->display, self->surface,
	                EGL_HEIGHT, &self->height);
	LOGI("%ix%i", self->width, self->height);

	// success
	return;

	// failure
	fail_current:
		eglDestroyContext(self->display, self->context);
		self->context = EGL_NO_CONTEXT;
	fail_context:
		eglDestroySurface(self->display, self->surface);
		self->surface = EGL_NO_SURFACE;
	fail_surface:
	fail_choose:
	fail_get_config:
		free(configs);
	fail_alloc_config:
	fail_num_config:
		eglTerminate(self->display);
	fail_initalize:
		self->display = EGL_NO_DISPLAY;
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

	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	eglSwapBuffers(self->display, self->surface);
}

static int platform_rendering(platform_t* self)
{
	assert(self);

	if(self->focus &&
	   (self->context != EGL_NO_CONTEXT))
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
