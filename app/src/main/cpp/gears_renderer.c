/*
 * Copyright (c) 2009-2010 Jeff Boody
 *
 * Gears for Android is a heavily modified port of the famous "gears" demo.
 * As such, it is a derived work subject to the license requirements (below)
 * of the original work.
 *
 * Copyright (c) 1999-2001  Brian Paul   All Rights Reserved.
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
#include <string.h>
#include <math.h>
#include "gears_renderer.h"
#include "a3d/widget/a3d_key.h"
#include "a3d/a3d_timestamp.h"
#include "libpak/pak_file.h"
#include "texgz/texgz_png.h"
#include "texgz/texgz_tex.h"

#define LOG_TAG "gears"
#include "a3d/a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

// gear colors
static const a3d_vec4f_t RED   = { .r=0.8f, .g=0.1f, .b=0.0f, .a=1.0f };
static const a3d_vec4f_t GREEN = { .r=0.0f, .g=0.8f, .b=0.2f, .a=1.0f };
static const a3d_vec4f_t BLUE  = { .r=0.2f, .g=0.2f, .b=1.0f, .a=1.0f };

static void gears_renderer_exit(gears_renderer_t* self)
{
	assert(self);

	gears_renderer_cmd_fn cmd_fn = self->cmd_fn;
	(*cmd_fn)(GEARS_CMD_EXIT, "");
}

static void gears_renderer_step(gears_renderer_t* self)
{
	assert(self);

	vkk_renderer_t* renderer;
	renderer = vkk_engine_renderer(self->engine);

	uint32_t width;
	uint32_t height;
	vkk_renderer_surfaceSize(renderer, &width, &height);

	// https://www.saschawillems.de/blog/2019/03/29/flipping-the-vulkan-viewport/
	// Vulkan uses a top-left left origin while OpenGL
	// uses a bottom-left origin so the frustum top and
	// bottom should be swapped to compensate
	float w = (float) width;
	float h = (float) height;
	if(h > w)
	{
		float a = h / w;
		a3d_mat4f_frustum(&self->pm, 1, -1.0f, 1.0f, a, -a, 5.0f, 60.0f);
	}
	else
	{
		float a = w / h;
		a3d_mat4f_frustum(&self->pm, 1, -a, a, 1.0f, -1.0f, 5.0f, 60.0f);
	}
	a3d_mat4f_translate(&self->mvm, 1, 0.0f, 0.0f, -40.0f);

	// glxgears: event_loop
	a3d_stack4f_push(self->mvm_stack, &self->mvm);
	a3d_mat4f_scale(&self->mvm, 0, self->view_scale, self->view_scale, self->view_scale);
	a3d_mat4f_rotateq(&self->mvm, 0, &self->view_q);

	// Gear1
	a3d_mat4f_t mvp;
	a3d_stack4f_push(self->mvm_stack, &self->mvm);
	a3d_mat4f_translate(&self->mvm, 0, -3.0f, -2.0f, 0.0f);
	a3d_mat4f_rotate(&self->mvm, 0, self->angle, 0.0f, 0.0f, 1.0f);
	a3d_mat4f_mulm_copy(&self->pm, &self->mvm, &mvp);
	gear_update(self->gear1, &mvp, &self->mvm);
	a3d_stack4f_pop(self->mvm_stack, &self->mvm);

	// Gear2
	a3d_stack4f_push(self->mvm_stack, &self->mvm);
	a3d_mat4f_translate(&self->mvm, 0, 3.1f, -2.0f, 0.0f);
	a3d_mat4f_rotate(&self->mvm, 0, -2.0f * self->angle - 9.0f, 0.0f, 0.0f, 1.0f);
	a3d_mat4f_mulm_copy(&self->pm, &self->mvm, &mvp);
	gear_update(self->gear2, &mvp, &self->mvm);
	a3d_stack4f_pop(self->mvm_stack, &self->mvm);

	// Gear3
	a3d_stack4f_push(self->mvm_stack, &self->mvm);
	a3d_mat4f_translate(&self->mvm, 0, -3.1f, 4.2f, 0.0f);
	a3d_mat4f_rotate(&self->mvm, 0, -2.0f * self->angle - 25.0f, 0.0f, 0.0f, 1.0f);
	a3d_mat4f_mulm_copy(&self->pm, &self->mvm, &mvp);
	gear_update(self->gear3, &mvp, &self->mvm);
	a3d_stack4f_pop(self->mvm_stack, &self->mvm);

	a3d_stack4f_pop(self->mvm_stack, &self->mvm);

	double t     = a3d_timestamp();
	double dt0   = t - self->t0;
	++self->frames;

	// don't update fps every frame
	if(dt0 >= 1.0)
	{
		float seconds  = (float) dt0;
		self->last_fps = ((float) self->frames)/seconds;

		//LOGI("%i frames in %.2lf seconds = %.2lf FPS", self->frames, seconds, self->last_fps);

		self->t0     = t;
		self->frames = 0;
	}

	// next frame
	if(self->t_last > 0.0)
	{
		float dt_last = (float) (t - self->t_last);

		if(self->last_fps > 30.0f)
		{
			dt_last = 1.0f/self->last_fps;
		}

		// make the gears rotate at a constant rate
		// red gear rotates at 1 revolution per 7 seconds
		self->angle += 360.0f * dt_last / 7.0f;
		if(self->angle >= 360.0f)
		{
			self->angle -= 360.0f;
		}
	}
	self->t_last = t;
}

static void gears_renderer_rotate(gears_renderer_t* self,
                                  float dx, float dy)
{
	assert(self);

	vkk_renderer_t* renderer;
	renderer = vkk_engine_renderer(self->engine);

	// TODO - vkk_renderer_surfaceSize shouldn't be called
	// outside beginn/end
	uint32_t width;
	uint32_t height;
	vkk_renderer_surfaceSize(renderer, &width, &height);

	// rotating around x-axis is equivalent to moving up-and-down on touchscreen
	// rotating around y-axis is equivalent to moving left-and-right on touchscreen
	// 360 degrees is equivalent to moving completly across the touchscreen
	float   w  = (float) width;
	float   h  = (float) height;
	GLfloat rx = 360.0f * dy / h;
	GLfloat ry = 360.0f * dx / w;
	a3d_quaternion_t q;
	a3d_quaternion_loadeuler(&q, rx, ry, 0.0f);
	a3d_quaternion_rotateq(&self->view_q, &q);
}

static void gears_renderer_scale(gears_renderer_t* self,
                                 float scale)
{
	assert(self);

	// scale range
	float min = 0.25f;
	float max = 2.0f;

	self->view_scale *= scale;
	if(self->view_scale < min)  self->view_scale = min;
	if(self->view_scale >= max) self->view_scale = max;
}

static int
gears_renderer_newUniformSetFactory(gears_renderer_t* self)
{
	assert(self);

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
			.sampler = self->sampler
		}
	};

	self->usf = vkk_engine_newUniformSetFactory(self->engine,
	                                            1, 4,
	                                            ub_array);
	if(self->usf == NULL)
	{
		return 0;
	}

	return 1;
}

static int
gears_renderer_newPipelineLayout(gears_renderer_t* self)
{
	assert(self);

	self->pl = vkk_engine_newPipelineLayout(self->engine,
	                                        1, &self->usf);
	if(self->pl == NULL)
	{
		return 0;
	}

	return 1;
}

static int
gears_renderer_newGraphicsPipeline(gears_renderer_t* self)
{
	assert(self);

	vkk_vertexBufferInfo_t vbi[2] =
	{
		// layout(location=0) in vec3 vertex;
		{
			.location   = 0,
			.components = 3,
			.format     = VKK_VERTEX_FORMAT_FLOAT
		},
		// layout(location=1) in vec3 normal;
		{
			.location   = 1,
			.components = 3,
			.format     = VKK_VERTEX_FORMAT_FLOAT
		}
	};

	vkk_graphicsPipelineInfo_t gpi =
	{
		.renderer          = vkk_engine_renderer(self->engine),
		.pl                = self->pl,
		.vs                = "vert.spv",
		.fs                = "frag.spv",
		.vb_count          = 2,
		.vbi               = vbi,
		.primitive         = VKK_PRIMITIVE_TRIANGLE_STRIP,
		.primitive_restart = 0,
		.cull_back         = 0,
		.depth_test        = 1,
		.depth_write       = 1,
		.blend_mode        = 0
	};

	self->gp = vkk_engine_newGraphicsPipeline(self->engine,
	                                          &gpi);
	if(self->gp == NULL)
	{
		return 0;
	}

	return 1;
}

static int
gears_renderer_newImage(gears_renderer_t* self)
{
	assert(self);

	pak_file_t* pak;
	pak = pak_file_open(GEARS_RESOURCE, PAK_FLAG_READ);
	if(pak == NULL)
	{
		return 0;
	}

	int size = pak_file_seek(pak, "lava.png");
	if(size == 0)
	{
		LOGE("invalid lava.png");
		pak_file_close(&pak);
		return 0;
	}

	texgz_tex_t* tex;
	tex = texgz_png_importf(pak->f, (size_t) size);
	if(tex == NULL)
	{
		pak_file_close(&pak);
		return 0;
	}
	pak_file_close(&pak);

	if(texgz_tex_convert(tex, TEXGZ_UNSIGNED_BYTE,
	                     TEXGZ_RGB) == 0)
	{
		goto fail_convert;
	}

	self->image = vkk_engine_newImage(self->engine,
	                                  (uint32_t) tex->width,
	                                  (uint32_t) tex->height,
	                                  VKK_IMAGE_FORMAT_RGB888,
	                                  1, VKK_STAGE_FS,
	                                  tex->pixels);
	if(self->image == NULL)
	{
		goto fail_image;
	}

	texgz_tex_delete(&tex);

	// success
	return 1;

	// failure
	fail_image:
	fail_convert:
		texgz_tex_delete(&tex);
	return 0;
}

static int
gears_renderer_newSampler(gears_renderer_t* self)
{
	assert(self);

	self->sampler = vkk_engine_newSampler(self->engine,
	                                      VKK_SAMPLER_FILTER_LINEAR,
	                                      VKK_SAMPLER_FILTER_LINEAR,
	                                      VKK_SAMPLER_MIPMAP_MODE_NEAREST);
	if(self->sampler == NULL)
	{
		return 0;
	}

	return 1;
}

/***********************************************************
* public                                                   *
***********************************************************/

gears_renderer_t*
gears_renderer_new(void* app,
                   const char* app_name,
                   uint32_t app_version,
                   gears_renderer_cmd_fn cmd_fn)
{
	#ifdef ANDROID
		assert(app);
	#else
		assert(app == NULL);
	#endif
	assert(app_name);

	gears_renderer_t* self;
	self = (gears_renderer_t*)
	       calloc(1, sizeof(gears_renderer_t));
	if(self == NULL)
	{
		LOGE("calloc failed");
		return NULL;
	}

	self->engine = vkk_engine_new(app, app_name,
	                              app_version,
	                              GEARS_RESOURCE,
	                              GEARS_CACHE);
	if(self->engine == NULL)
	{
		goto fail_engine;
	}

	if(gears_renderer_newSampler(self) == 0)
	{
		goto fail_sampler;
	}

	if(gears_renderer_newUniformSetFactory(self) == 0)
	{
		goto fail_usf;
	}

	if(gears_renderer_newPipelineLayout(self) == 0)
	{
		goto fail_pl;
	}

	if(gears_renderer_newGraphicsPipeline(self) == 0)
	{
		goto fail_gp;
	}

	if(gears_renderer_newImage(self) == 0)
	{
		goto fail_image;
	}

	self->view_scale = 1.0f;

	a3d_quaternion_t qx;
	a3d_quaternion_t qy;
	a3d_quaternion_loadeuler(&qx, 20.0f,  0.0f, 0.0f);
	a3d_quaternion_loadeuler(&qy,  0.0f, 30.0f, 0.0f);
	a3d_quaternion_rotateq_copy(&qy, &qx, &self->view_q);

	self->angle = 0.0f;

	a3d_mat4f_identity(&self->pm);
	a3d_mat4f_identity(&self->mvm);

	self->t0       = a3d_timestamp();
	self->t_last   = 0.0;
	self->frames   = 0;
	self->last_fps = 0.0f;

	// create the gears buffers
	self->gear1 = gear_new(self, &RED,
	                       1.0f, 4.0f, 1.0f, 20, 0.7f);
	if(self->gear1 == NULL)
	{
		goto fail_gear1;
	}

	self->gear2 = gear_new(self, &GREEN,
	                       0.5f, 2.0f, 2.0f, 10, 0.7f);
	if(self->gear2 == NULL)
	{
		goto fail_gear2;
	}

	self->gear3 = gear_new(self, &BLUE,
	                       1.3f, 2.0f, 0.5f, 10, 0.7f);
	if(self->gear3 == NULL)
	{
		goto fail_gear3;
	}

	self->mvm_stack = a3d_stack4f_new();
	if(self->mvm_stack == NULL)
	{
		goto fail_stack;
	}

	self->cmd_fn = cmd_fn;

	// success
	return self;

	// failure
	fail_stack:
		gear_delete(&self->gear3);
	fail_gear3:
		gear_delete(&self->gear2);
	fail_gear2:
		gear_delete(&self->gear1);
	fail_gear1:
		vkk_engine_deleteImage(self->engine,
		                       &self->image);
	fail_image:
		vkk_engine_deleteGraphicsPipeline(self->engine,
		                                  &self->gp);
	fail_gp:
		vkk_engine_deletePipelineLayout(self->engine,
		                                &self->pl);
	fail_pl:
		vkk_engine_deleteUniformSetFactory(self->engine,
		                                   &self->usf);
	fail_usf:
		vkk_engine_deleteSampler(self->engine,
		                         &self->sampler);
	fail_sampler:
		vkk_engine_delete(&self->engine);
	fail_engine:
		free(self);
	return NULL;
}

void gears_renderer_delete(gears_renderer_t** _self)
{
	// *_self can be null
	assert(_self);

	gears_renderer_t* self = *_self;
	if(self)
	{
		vkk_engine_shutdown(self->engine);

		gear_delete(&self->gear3);
		gear_delete(&self->gear2);
		gear_delete(&self->gear1);
		a3d_stack4f_delete(&self->mvm_stack);

		vkk_engine_deleteImage(self->engine,
		                       &self->image);
		vkk_engine_deleteGraphicsPipeline(self->engine,
		                                  &self->gp);
		vkk_engine_deletePipelineLayout(self->engine,
		                                &self->pl);
		vkk_engine_deleteUniformSetFactory(self->engine,
		                                   &self->usf);
		vkk_engine_deleteSampler(self->engine,
		                         &self->sampler);
		vkk_engine_delete(&self->engine);
		free(self);
		*_self = NULL;
	}
}

void gears_renderer_draw(gears_renderer_t* self)
{
	assert(self);

	vkk_renderer_t* renderer;
	renderer = vkk_engine_renderer(self->engine);

	float clear_color[4] =
	{
		0.0f, 0.0f, 0.0f, 1.0f
	};
	if(vkk_renderer_begin(renderer, clear_color) == 0)
	{
		return;
	}

	gears_renderer_step(self);

	vkk_renderer_bindGraphicsPipeline(renderer, self->gp);

	gear_draw(self->gear1);
	gear_draw(self->gear2);
	gear_draw(self->gear3);

	vkk_renderer_end(renderer);
}

int gears_renderer_resize(gears_renderer_t* self)
{
	assert(self);

	return vkk_engine_resize(self->engine);
}

void gears_renderer_touch(gears_renderer_t* self,
                          int action, int count, double ts,
                          float x0, float y0,
                          float x1, float y1,
                          float x2, float y2,
                          float x3, float y3)
{
	assert(self);

	if(action == GEARS_TOUCH_ACTION_UP)
	{
		// Do nothing
		self->touch_state = GEARS_TOUCH_STATE_INIT;
	}
	else if(count == 1)
	{
		if(self->touch_state == GEARS_TOUCH_STATE_ROTATE)
		{
			float dx = x0 - self->touch_x1;
			float dy = y0 - self->touch_y1;
			gears_renderer_rotate(self, dx, dy);
			self->touch_x1 = x0;
			self->touch_y1 = y0;
		}
		else if(action == GEARS_TOUCH_ACTION_DOWN)
		{
			self->touch_x1    = x0;
			self->touch_y1    = y0;
			self->touch_state = GEARS_TOUCH_STATE_ROTATE;
		}
	}
	else if(count == 2)
	{
		if(self->touch_state == GEARS_TOUCH_STATE_ZOOM)
		{
			float dx = fabsf(x1 - x0);
			float dy = fabsf(y1 - y0);
			float ds = sqrtf(dx*dx + dy*dy);

			gears_renderer_scale(self, ds/self->touch_ds);

			self->touch_ds = ds;
		}
		else
		{
			float dx = fabsf(x1 - x0);
			float dy = fabsf(y1 - y0);
			float ds = sqrtf(dx*dx + dy*dy);

			self->touch_ds    = ds;
			self->touch_state = GEARS_TOUCH_STATE_ZOOM;
		}
	}
}

void gears_renderer_keyPress(gears_renderer_t* self,
                             int keycode, int meta)
{
	assert(self);

	if(keycode == A3D_KEY_ESCAPE)
	{
		// double tap back to exit
		double t1 = a3d_timestamp();
		if((t1 - self->escape_t0) < 0.5)
		{
			gears_renderer_exit(self);
		}
		else
		{
			self->escape_t0 = t1;
		}
	}
}
