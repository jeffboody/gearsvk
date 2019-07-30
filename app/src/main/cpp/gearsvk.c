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
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

#ifdef ANDROID
	#include <vulkan_wrapper.h>
	#include <android_native_app_glue.h>
#else
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_vulkan.h>
	#include <vulkan/vulkan.h>
#endif

#include "a3d/a3d_timestamp.h"
#include "a3d/widget/a3d_key.h"
#include "libcc/cc_list.h"
#include "libcc/cc_timestamp.h"
#include "gears_renderer.h"

#define LOG_TAG "gears"
#include "a3d/a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static int                      gRunning      = 1;
static vkk_engine_t*            engine        = NULL;
static vkk_uniformSetFactory_t* usf           = NULL;
static vkk_sampler_t*           sampler       = NULL;
static cc_list_t*               renderer_list = NULL;
static cc_list_t*               create_list   = NULL;
static cc_list_t*               destroy_list  = NULL;
static pthread_mutex_t          mutex         = PTHREAD_MUTEX_INITIALIZER;;
static pthread_cond_t           cond          = PTHREAD_COND_INITIALIZER;

static void cmd_fn(int cmd, const char* msg)
{
	if(cmd == GEARS_CMD_EXIT)
	{
		pthread_mutex_lock(&mutex);
		gRunning = 0;
		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&mutex);
	}
}

static int keyPress(SDL_Keysym* keysym,
                    int* keycode, int* meta)
{
	assert(keysym);
	assert(keycode);
	assert(meta);

	// convert the keycode
	// TODO - INSERT
	*keycode = 0;
	*meta    = 0;
	if((keysym->sym >= SDLK_a) &&
	   (keysym->sym <= SDLK_z))
	{
		int a = (int) 'a';
		*keycode = a + (keysym->sym - SDLK_a);
	}
	else if((keysym->sym >= SDLK_0) &&
	        (keysym->sym <= SDLK_9))
	{
		int z = (int) '0';
		*keycode = z + (keysym->sym - SDLK_0);
	}
	else if((keysym->sym >= SDLK_KP_0) &&
	        (keysym->sym <= SDLK_KP_9))
	{
		int z = (int) '0';
		*keycode = z + (keysym->sym - SDLK_KP_0);
	}
	else if((keysym->sym == SDLK_RETURN) ||
	        (keysym->sym == SDLK_KP_ENTER))
	{
		*keycode = A3D_KEY_ENTER;
	}
	else if(keysym->sym == SDLK_ESCAPE)
	{
		*keycode = A3D_KEY_ESCAPE;
	}
	else if(keysym->sym == SDLK_BACKSPACE)
	{
		*keycode = A3D_KEY_BACKSPACE;
	}
	else if(keysym->sym == SDLK_DELETE)
	{
		*keycode = A3D_KEY_DELETE;
	}
	else if(keysym->sym == SDLK_UP)
	{
		*keycode = A3D_KEY_UP;
	}
	else if(keysym->sym == SDLK_DOWN)
	{
		*keycode = A3D_KEY_DOWN;
	}
	else if(keysym->sym == SDLK_LEFT)
	{
		*keycode = A3D_KEY_LEFT;
	}
	else if(keysym->sym == SDLK_RIGHT)
	{
		*keycode = A3D_KEY_RIGHT;
	}
	else if(keysym->sym == SDLK_HOME)
	{
		*keycode = A3D_KEY_HOME;
	}
	else if(keysym->sym == SDLK_END)
	{
		*keycode = A3D_KEY_END;
	}
	else if(keysym->sym == SDLK_PAGEUP)
	{
		*keycode = A3D_KEY_PGUP;
	}
	else if(keysym->sym == SDLK_PAGEDOWN)
	{
		*keycode = A3D_KEY_PGDOWN;
	}
	else if(keysym->sym == SDLK_ESCAPE)
	{
		*keycode = A3D_KEY_ESCAPE;
	}
	else if(keysym->sym == SDLK_BACKSPACE)
	{
		*keycode = (int) '\b';
	}
	else if(keysym->sym == SDLK_TAB)
	{
		*keycode = (int) '\t';
	}
	else if(keysym->sym == SDLK_SPACE)
	{
		*keycode = (int) ' ';
	}
	else if(keysym->sym == SDLK_EXCLAIM)
	{
		*keycode = (int) '!';
	}
	else if(keysym->sym == SDLK_QUOTEDBL)
	{
		*keycode = (int) '"';
	}
	else if(keysym->sym == SDLK_HASH)
	{
		*keycode = (int) '#';
	}
	else if(keysym->sym == SDLK_DOLLAR)
	{
		*keycode = (int) '$';
	}
	else if(keysym->sym == SDLK_AMPERSAND)
	{
		*keycode = (int) '&';
	}
	else if(keysym->sym == SDLK_QUOTE)
	{
		*keycode = (int) '\'';
	}
	else if(keysym->sym == SDLK_LEFTPAREN)
	{
		*keycode = (int) '(';
	}
	else if(keysym->sym == SDLK_RIGHTPAREN)
	{
		*keycode = (int) ')';
	}
	else if((keysym->sym == SDLK_ASTERISK) ||
	        (keysym->sym == SDLK_KP_MULTIPLY))
	{
		*keycode = (int) '*';
	}
	else if((keysym->sym == SDLK_PLUS) ||
	        (keysym->sym == SDLK_KP_PLUS))
	{
		*keycode = (int) '+';
	}
	else if(keysym->sym == SDLK_COMMA)
	{
		*keycode = (int) ',';
	}
	else if((keysym->sym == SDLK_MINUS) ||
	        (keysym->sym == SDLK_KP_MINUS))
	{
		*keycode = (int) '-';
	}
	else if((keysym->sym == SDLK_PERIOD) ||
	        (keysym->sym == SDLK_KP_PERIOD))
	{
		*keycode = (int) '.';
	}
	else if((keysym->sym == SDLK_SLASH) ||
	        (keysym->sym == SDLK_KP_DIVIDE))
	{
		*keycode = (int) '/';
	}
	else if(keysym->sym == SDLK_COLON)
	{
		*keycode = (int) ':';
	}
	else if(keysym->sym == SDLK_SEMICOLON)
	{
		*keycode = (int) ';';
	}
	else if(keysym->sym == SDLK_LESS)
	{
		*keycode = (int) '<';
	}
	else if((keysym->sym == SDLK_EQUALS) ||
	        (keysym->sym == SDLK_KP_EQUALS))
	{
		*keycode = (int) '=';
	}
	else if(keysym->sym == SDLK_GREATER)
	{
		*keycode = (int) '>';
	}
	else if(keysym->sym == SDLK_QUESTION)
	{
		*keycode = (int) '?';
	}
	else if(keysym->sym == SDLK_AT)
	{
		*keycode = (int) '@';
	}
	else if(keysym->sym == SDLK_LEFTBRACKET)
	{
		*keycode = (int) '[';
	}
	else if(keysym->sym == SDLK_BACKSLASH)
	{
		*keycode = (int) '\\';
	}
	else if(keysym->sym == SDLK_RIGHTBRACKET)
	{
		*keycode = (int) ']';
	}
	else if(keysym->sym == SDLK_CARET)
	{
		*keycode = (int) '^';
	}
	else if(keysym->sym == SDLK_UNDERSCORE)
	{
		*keycode = (int) '_';
	}
	else if(keysym->sym == SDLK_BACKQUOTE)
	{
		*keycode = (int) '`';
	}
	else if(keysym->sym == SDLK_BACKQUOTE)
	{
		*keycode = (int) '`';
	}
	else
	{
		// ignore
		LOGD("sym=0x%X", keysym->sym);
		return 0;
	}

	// convert the meta
	if(keysym->mod & KMOD_ALT)
	{
		*meta |= A3D_KEY_ALT;
	}
	if(keysym->mod & KMOD_LALT)
	{
		*meta |= A3D_KEY_ALT_L;
	}
	if(keysym->mod & KMOD_RALT)
	{
		*meta |= A3D_KEY_ALT_R;
	}
	if(keysym->mod & KMOD_CTRL)
	{
		*meta |= A3D_KEY_CTRL;
	}
	if(keysym->mod & KMOD_LCTRL)
	{
		*meta |= A3D_KEY_CTRL_L;
	}
	if(keysym->mod & KMOD_RCTRL)
	{
		*meta |= A3D_KEY_CTRL_R;
	}
	if(keysym->mod & KMOD_SHIFT)
	{
		*meta |= A3D_KEY_SHIFT;
	}
	if(keysym->mod & KMOD_LSHIFT)
	{
		*meta |= A3D_KEY_SHIFT_L;
	}
	if(keysym->mod & KMOD_RSHIFT)
	{
		*meta |= A3D_KEY_SHIFT_R;
	}
	if(keysym->mod & KMOD_CAPS)
	{
		*meta |= A3D_KEY_CAPS;
	}

	return 1;
}

/***********************************************************
* threads                                                  *
***********************************************************/

static int
gearsvk_newUniformSetFactory(void)
{
	vkk_uniformBinding_t ub_array[4] =
	{
		// layout(std140, set=0, binding=0) uniform uniformMvp
		// {
		//     mat4 mvp;
		// };
		{
			.binding = 0,
			.type    = VKK_UNIFORM_TYPE_BUFFER,
			.stage   = VKK_STAGE_VS,
			.sampler = NULL
		},
		// layout(std140, set=0, binding=1) uniform uniformNm
		// {
		//     mat4 nm;
		// };
		{
			.binding = 1,
			.type    = VKK_UNIFORM_TYPE_BUFFER,
			.stage   = VKK_STAGE_VS,
			.sampler = NULL
		},
		// layout(std140, set=0, binding=2) uniform uniformColor
		// {
		//     vec4 color;
		// };
		{
			.binding = 2,
			.type    = VKK_UNIFORM_TYPE_BUFFER,
			.stage   = VKK_STAGE_FS,
			.sampler = NULL
		},
		// layout(set=0, binding=3) uniform sampler2D lava_sampler;
		{
			.binding = 3,
			.type    = VKK_UNIFORM_TYPE_SAMPLER,
			.stage   = VKK_STAGE_FS,
			.sampler = sampler
		}
	};

	usf = vkk_engine_newUniformSetFactory(engine,
	                                      1, 4,
	                                      ub_array);
	if(usf == NULL)
	{
		return 0;
	}

	return 1;
}

static int
gearsvk_newSampler(void)
{
	sampler = vkk_engine_newSampler(engine,
	                                VKK_SAMPLER_FILTER_LINEAR,
	                                VKK_SAMPLER_FILTER_LINEAR,
	                                VKK_SAMPLER_MIPMAP_MODE_NEAREST);
	if(sampler == NULL)
	{
		return 0;
	}

	return 1;
}

static void* gearsvk_create(void* arg)
{
	pthread_mutex_lock(&mutex);
	while(gRunning)
	{
		while(cc_list_size(create_list) < 10)
		{
			pthread_mutex_unlock(&mutex);

			gears_renderer_t* renderer;
			float distance = -3.0f + ((float) (rand()%61))/10.0f;
			float scale    = ((float) (rand()%21))/10.0f;
			renderer = gears_renderer_new(engine, usf, sampler, distance, scale, cmd_fn);

			pthread_mutex_lock(&mutex);
			if(renderer)
			{
				cc_list_append(create_list, NULL, (const void*) renderer);
			}
			else
			{
				break;
			}
		}

		pthread_cond_wait(&cond, &mutex);
	}
	pthread_mutex_unlock(&mutex);

	return NULL;
}

static void* gearsvk_destroy(void* arg)
{
	pthread_mutex_lock(&mutex);
	while(gRunning)
	{
		cc_listIter_t* iter = cc_list_head(destroy_list);
		while(iter)
		{
			gears_renderer_t* renderer;
			renderer = (gears_renderer_t*)
			           cc_list_remove(destroy_list, &iter);
			pthread_mutex_unlock(&mutex);

			gears_renderer_delete(&renderer);

			pthread_mutex_lock(&mutex);
		}

		pthread_cond_wait(&cond, &mutex);
	}
	pthread_mutex_unlock(&mutex);

	return NULL;
}

/***********************************************************
* main                                                     *
***********************************************************/

int main(int argc, char** argv)
{
	uint32_t version = VK_MAKE_VERSION(1,0,0);
	engine = vkk_engine_new(NULL, "GearsVK",
	                        version,
	                        GEARS_RESOURCE,
	                        GEARS_CACHE);
	if(engine == NULL)
	{
		return EXIT_FAILURE;
	}

	if(gearsvk_newSampler() == 0)
	{
		return EXIT_FAILURE;
	}

	if(gearsvk_newUniformSetFactory() == 0)
	{
		return EXIT_FAILURE;
	}

	renderer_list = cc_list_new();
	if(renderer_list == NULL)
	{
		return EXIT_FAILURE;
	}

	create_list = cc_list_new();
	if(create_list == NULL)
	{
		return EXIT_FAILURE;
	}

	destroy_list = cc_list_new();
	if(destroy_list == NULL)
	{
		return EXIT_FAILURE;
	}

	// PTHREAD_MUTEX_DEFAULT is not re-entrant
	if(pthread_mutex_init(&mutex, NULL) != 0)
	{
		LOGE("pthread_mutex_init failed");
		return EXIT_FAILURE;
	}

	if(pthread_cond_init(&cond, NULL) != 0)
	{
		LOGE("pthread_cond_init failed");
		return EXIT_FAILURE;
	}

	pthread_t thread_create;
	if(pthread_create(&thread_create, NULL,
	                  gearsvk_create,
	                  (void*) NULL) != 0)
	{
		LOGE("pthread_create failed");
		return EXIT_FAILURE;
	}

	pthread_t thread_destroy;
	if(pthread_create(&thread_destroy, NULL,
	                  gearsvk_destroy,
	                  (void*) NULL) != 0)
	{
		LOGE("pthread_create failed");
		return EXIT_FAILURE;
	}

	double t0 = 0;
	cc_listIter_t* iter;
	while(gRunning)
	{
		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
			int keycode = 0;
			int meta    = 0;
			if((e.type == SDL_KEYUP) ||
			   ((e.type == SDL_KEYDOWN) && (e.key.repeat)))
			{
				if(keyPress(&e.key.keysym, &keycode, &meta))
				{
					iter = cc_list_head(renderer_list);
					while(iter)
					{
						gears_renderer_t* renderer;
						renderer = (gears_renderer_t*)
						           cc_list_peekIter(iter);
						gears_renderer_keyPress(renderer,
						                        keycode, meta);
						iter = cc_list_next(iter);
					}
				}
			}
			else if(e.type == SDL_MOUSEBUTTONUP)
			{
				iter = cc_list_head(renderer_list);
				while(iter)
				{
					gears_renderer_t* renderer;
					renderer = (gears_renderer_t*)
					           cc_list_peekIter(iter);
					float  x  = (float) e.button.x;
					float  y  = (float) e.button.y;
					double ts = a3d_timestamp();
					gears_renderer_touch(renderer,
					                     GEARS_TOUCH_ACTION_UP,
					                     1, ts, x, y,
					                     0.0f, 0.0f,
					                     0.0f, 0.0f,
					                     0.0f, 0.0f);
					iter = cc_list_next(iter);
				}
			}
			else if(e.type == SDL_MOUSEBUTTONDOWN)
			{
				iter = cc_list_head(renderer_list);
				while(iter)
				{
					gears_renderer_t* renderer;
					renderer = (gears_renderer_t*)
					           cc_list_peekIter(iter);
					float  x  = (float) e.button.x;
					float  y  = (float) e.button.y;
					double ts = a3d_timestamp();
					gears_renderer_touch(renderer,
					                     GEARS_TOUCH_ACTION_DOWN,
					                     1, ts, x, y,
					                     0.0f, 0.0f,
					                     0.0f, 0.0f,
					                     0.0f, 0.0f);
					iter = cc_list_next(iter);
				}
			}
			else if(e.type == SDL_MOUSEMOTION)
			{
				iter = cc_list_head(renderer_list);
				while(iter)
				{
					gears_renderer_t* renderer;
					renderer = (gears_renderer_t*)
					           cc_list_peekIter(iter);
					float  x  = (float) e.button.x;
					float  y  = (float) e.button.y;
					double ts = a3d_timestamp();
					gears_renderer_touch(renderer,
					                     GEARS_TOUCH_ACTION_MOVE,
					                     1, ts, x, y,
					                     0.0f, 0.0f,
					                     0.0f, 0.0f,
					                     0.0f, 0.0f);
					iter = cc_list_next(iter);
				}
			}
			else if(e.type == SDL_WINDOWEVENT)
			{
				if(e.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					if(vkk_engine_resize(engine) == 0)
					{
						return EXIT_FAILURE;
					}
				}
			}
			else if(e.type == SDL_QUIT)
			{
				pthread_mutex_lock(&mutex);
				gRunning = 0;
				pthread_cond_broadcast(&cond);
				pthread_mutex_unlock(&mutex);
			}
		}

		vkk_renderer_t* rend;
		rend = vkk_engine_renderer(engine);

		float clear_color[4] =
		{
			0.0f, 0.0f, 0.0f, 1.0f
		};
		if(vkk_renderer_beginDefault(rend, clear_color))
		{
			iter = cc_list_head(renderer_list);
			while(iter)
			{
				gears_renderer_t* renderer;
				renderer = (gears_renderer_t*)
				           cc_list_peekIter(iter);
				gears_renderer_draw(renderer);
				iter = cc_list_next(iter);
			}
			vkk_renderer_end(rend);
		}

		// swap the objects
		double t1 = cc_timestamp();
		if((t1 - t0 > 3.0) ||
		   (cc_list_size(renderer_list) == 0))
		{
			t0 = t1;
			pthread_mutex_lock(&mutex);
			cc_list_appendList(destroy_list,
			                   renderer_list);
			cc_list_appendList(renderer_list,
			                   create_list);
			pthread_cond_broadcast(&cond);
			pthread_mutex_unlock(&mutex);
		}
	}

	vkk_engine_shutdown(engine);
	pthread_join(thread_create, NULL);
	pthread_join(thread_destroy, NULL);

	iter = cc_list_head(destroy_list);
	while(iter)
	{
		gears_renderer_t* renderer;
		renderer = (gears_renderer_t*)
		           cc_list_remove(destroy_list, &iter);
		gears_renderer_delete(&renderer);
	}

	iter = cc_list_head(create_list);
	while(iter)
	{
		gears_renderer_t* renderer;
		renderer = (gears_renderer_t*)
		           cc_list_remove(create_list, &iter);
		gears_renderer_delete(&renderer);
	}

	iter = cc_list_head(renderer_list);
	while(iter)
	{
		gears_renderer_t* renderer;
		renderer = (gears_renderer_t*)
		           cc_list_remove(renderer_list, &iter);
		gears_renderer_delete(&renderer);
	}

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
	cc_list_delete(&destroy_list);
	cc_list_delete(&create_list);
	cc_list_delete(&renderer_list);
	vkk_engine_deleteUniformSetFactory(engine, &usf);
	vkk_engine_deleteSampler(engine, &sampler);
	vkk_engine_delete(&engine);

	return EXIT_SUCCESS;
}
