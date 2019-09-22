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

#include <android_native_app_glue.h>
#include <assert.h>
#include <jni.h>
#include <stdlib.h>
#include <unistd.h>
#include <vulkan_wrapper.h>

#define LOG_TAG "gears"
#include "libcc/cc_log.h"
#include "libvkk/vkui/vkui_key.h"
#include "gears_renderer.h"

/***********************************************************
* private - cmd_fn callback                                *
***********************************************************/

// java state required for cmd_fn
static JavaVM* g_vm = NULL;
static jobject g_clazz;
static float   g_density = 1.0f;

static void cmd_fn(int cmd, const char* msg)
{
	if(g_vm == NULL)
	{
		LOGE("g_vm is NULL");
		return;
	}

	JNIEnv* env = NULL;
	if((*g_vm)->AttachCurrentThread(g_vm, &env, NULL) != 0)
	{
		LOGE("AttachCurrentThread failed");
		return;
	}

	jclass cls = (*env)->GetObjectClass(env, g_clazz);
	if(cls == NULL)
	{
		LOGE("FindClass failed");
		return;
	}

	jmethodID mid = (*env)->GetStaticMethodID(env, cls,
	                                          "CallbackCmd",
	                                          "(ILjava/lang/String;)V");
	if(mid == NULL)
	{
		LOGE("GetStaticMethodID failed");
		return;
	}

	jstring jmsg = (*env)->NewStringUTF(env, msg);
	if(jmsg == NULL)
	{
		LOGE("NewStringUTF failed");
		return;
	}

	(*env)->CallStaticVoidMethod(env, cls, mid, cmd, jmsg);
	(*env)->DeleteLocalRef(env, jmsg);
}

/***********************************************************
* JNI interface                                            *
***********************************************************/

JNIEXPORT void JNICALL
Java_com_jeffboody_gearsvk_GearsVK_NativeChangeDensity(JNIEnv* env,
                                                       jobject obj,
                                                       jfloat density)
{
	assert(env);

	g_density = density;
}

/***********************************************************
* platform                                                 *
***********************************************************/

typedef struct
{
	struct android_app* app;
	int focus;

	int     drawable;
	int32_t width;
	int32_t height;
	float   density;

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

	g_vm    = app->activity->vm;
	g_clazz = app->activity->clazz;

	return self;
}

static void
platform_termWindow(platform_t* self)
{
	assert(self);

	gears_renderer_delete(&self->renderer);
}

static int
platform_isTimestampValid(platform_t* self)
{
	assert(self);

	// import ts1
	AAssetManager* am;
	am = self->app->activity->assetManager;
	AAsset* asset = AAssetManager_open(am, "timestamp.raw",
	                                   AASSET_MODE_BUFFER);
	if(asset == NULL)
	{
		LOGE("AAssetManager_open %s failed", "timestamp.raw");
		return 0;
	}

	size_t sz1 = (size_t) AAsset_getLength(asset);
	if(sz1 == 0)
	{
		LOGE("invalid sz1=%u", (unsigned int) sz1);
		goto fail_ts1_size;
	}

	char* ts1 = (char*) calloc(sz1, sizeof(char));
	if(ts1 == NULL)
	{
		LOGE("calloc failed");
		goto fail_ts1_alloc;
	}

	if(AAsset_read(asset, ts1, sz1) != sz1)
	{
		LOGE("AAsset_read failed");
		goto fail_ts1_read;
	}

	// check if ts2 exists
	if(access(GEARS_TIMESTAMP, F_OK) != 0)
	{
		LOGW("invalid %s", GEARS_TIMESTAMP);
		goto fail_ts2_access;
	}

	// import ts2
	FILE* f = fopen(GEARS_TIMESTAMP, "r");
	if(f == NULL)
	{
		LOGW("invalid %s", GEARS_TIMESTAMP);
		goto fail_ts2_fopen;
	}

	fseek(f, (long) 0, SEEK_END);
	size_t sz2 = (size_t) ftell(f);
	fseek(f, 0, SEEK_SET);

	if(sz1 != sz2)
	{
		LOGW("invalid sz1=%i, sz2=%i", (int) sz1, (int) sz2);
		goto fail_compare_size;
	}

	char* ts2 = (char*) calloc(sz2, sizeof(char));
	if(ts2 == NULL)
	{
		LOGE("calloc failed");
		goto fail_ts2_alloc;
	}

	if(fread(ts2, sz2, 1, f) != 1)
	{
		LOGE("fread failed");
		goto fail_ts2_read;
	}

	// compare timestamps
	for(int i = 0; i < sz1; ++i)
	{
		if(ts1[i] != ts2[i])
		{
			LOGW("invalid ts1=%s, ts2=%s", ts1, ts2);
			goto fail_compare;
		}
	}

	free(ts2);
	fclose(f);
	free(ts1);
	AAsset_close(asset);

	// success
	return 1;

	// failure
	// TODO
	fail_compare:
	fail_ts2_read:
		free(ts2);
	fail_ts2_alloc:
	fail_compare_size:
		fclose(f);
	fail_ts2_fopen:
	fail_ts2_access:
	fail_ts1_read:
		free(ts1);
	fail_ts1_alloc:
	fail_ts1_size:
		AAsset_close(asset);
	return 0;
}

static int
platform_updateResource(platform_t* self, const char* src,
                        const char* dst)
{
	assert(self);
	assert(src);
	assert(dst);

	// remove old resource
	unlink(dst);

	// import resource
	AAssetManager* am;
	am = self->app->activity->assetManager;
	AAsset* asset = AAssetManager_open(am, src,
	                                   AASSET_MODE_BUFFER);
	if(asset == NULL)
	{
		LOGE("AAssetManager_open %s failed", src);
		return 0;
	}

	size_t size = (size_t) AAsset_getLength(asset);
	if(size == 0)
	{
		LOGE("invalid size=%u", (unsigned int) size);
		goto fail_size;
	}

	char* buf;
	buf = (char*) calloc(size, sizeof(char));
	if(buf == NULL)
	{
		LOGE("calloc failed");
		goto fail_alloc;
	}

	if(AAsset_read(asset, buf, size) != size)
	{
		LOGE("AAsset_read failed");
		goto fail_read;
	}

	// export resource
	FILE* f = fopen(dst, "w");
	if(f == NULL)
	{
		LOGE("fopen %s failed", dst);
		goto fail_fopen;
	}

	if(fwrite(buf, size, 1, f) != 1)
	{
		LOGE("fwrite %s failed", dst);
		goto fail_fwrite;
	}

	fclose(f);
	free(buf);
	AAsset_close(asset);

	// success
	return 1;

	// failure
	fail_fwrite:
		fclose(f);
	fail_fopen:
	fail_read:
		free(buf);
	fail_alloc:
	fail_size:
		AAsset_close(asset);
	return 0;
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

	if(platform_isTimestampValid(self) == 0)
	{
		if(platform_updateResource(self, "resource.pak",
		                           GEARS_RESOURCE) == 0)
		{
			return;
		}

		if(platform_updateResource(self, "timestamp.raw",
		                           GEARS_TIMESTAMP) == 0)
		{
			return;
		}
	}

	LOGI("InitVulkan=%i", InitVulkan());

	uint32_t version = VKK_MAKE_VERSION(1,0,0);
	self->renderer = gears_renderer_new(self->app,
	                                    "gearsvk",
	                                    version,
	                                    cmd_fn);
	if(self->renderer == NULL)
	{
		return;
	}

	self->drawable = 1;
	self->width    = ANativeWindow_getWidth(self->app->window);
	self->height   = ANativeWindow_getHeight(self->app->window);
	self->density  = 1.0f;
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
		// check if the native window was resized
		int32_t width;
		int32_t height;
		width  = ANativeWindow_getWidth(self->app->window);
		height = ANativeWindow_getHeight(self->app->window);
		if((self->width  != width) ||
		   (self->height != height))
		{
			if(gears_renderer_resize(self->renderer) == 0)
			{
				self->drawable = 0;
				return;
			}
			self->drawable = 1;
			self->width    = width;
			self->height   = height;
		}

		if(self->density != g_density)
		{
			self->density = g_density;
			gears_renderer_density(self->renderer, g_density);
		}

		if(self->drawable)
		{
			gears_renderer_draw(self->renderer);
		}
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
	assert(app);
	assert(event);

	platform_t*       platform = (platform_t*) app->userData;
	gears_renderer_t* renderer = platform->renderer;

	int32_t type = AInputEvent_getType(event);
	if(type == AINPUT_EVENT_TYPE_KEY)
	{
		int32_t action  = AKeyEvent_getAction(event);
		int32_t keycode = AKeyEvent_getKeyCode(event);

		if((action == AKEY_EVENT_ACTION_UP) &&
		   (keycode == AKEYCODE_BACK))
		{
			gears_renderer_keyPress(renderer,
			                        VKUI_KEY_ESCAPE, 0);
		}
		return 1;
	}
	else if(type == AINPUT_EVENT_TYPE_MOTION)
	{
		int     action = (int) AMotionEvent_getAction(event) &
		                       AMOTION_EVENT_ACTION_MASK;
		int64_t ns     = AMotionEvent_getEventTime(event);
		double  ts     = ((double) ns)/1000000000.0;
		int     count  = (int) AMotionEvent_getPointerCount(event);

		if(count < 1)
		{
			// ignore
			return 0;
		}

		// get points
		float x0;
		float y0;
		float x1;
		float y1;
		float x2;
		float y2;
		float x3;
		float y3;
		if(count >= 1)
		{
			x0 = AMotionEvent_getX(event, 0);
			y0 = AMotionEvent_getY(event, 0);
		}
		if(count >= 2)
		{
			x1 = AMotionEvent_getX(event, 1);
			y1 = AMotionEvent_getY(event, 1);
		}
		if(count >= 3)
		{
			x2 = AMotionEvent_getX(event, 2);
			y2 = AMotionEvent_getY(event, 2);
		}
		if(count >= 4)
		{
			count = 4;
			x3 = AMotionEvent_getX(event, 3);
			y3 = AMotionEvent_getY(event, 3);
		}

		// translate the action
		if(action <= AMOTION_EVENT_ACTION_DOWN)
		{
			action = GEARS_TOUCH_ACTION_DOWN;
		}
		else if(action == AMOTION_EVENT_ACTION_UP)
		{
			action = GEARS_TOUCH_ACTION_UP;
		}
		else if(action == AMOTION_EVENT_ACTION_MOVE)
		{
			action = GEARS_TOUCH_ACTION_MOVE;
		}
		else
		{
			// ignore
			return 0;
		}

		gears_renderer_touch(renderer,
		                     action, count, ts,
		                     x0, y0,
		                     x1, y1,
		                     x2, y2,
		                     x3, y3);
		return 1;
	}

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
